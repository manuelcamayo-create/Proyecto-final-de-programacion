// ============================================================================
// CRITERIO 3 (Sockets TCP/UDP & Red Distribuida) - CAPA DE COMPATIBILIDAD
// Funciona en Windows (Winsock2, MinGW + Zinjal) y en Linux/POSIX.
//  - Windows: se enlaza con  -lws2_32   y se inicializa con WSAStartup()
//  - Linux:   no requiere librerias extra
// DescriptorSocket es un envoltorio RAII: cierra el socket automaticamente.
// IMPORTANTE: en main.cpp este archivo se incluye PRIMERO (winsock2.h debe ir antes
// que cualquier otra cabecera de Windows).
// ============================================================================
#ifndef SOCKET_UTIL_H
#define SOCKET_UTIL_H

#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
    #endif
    #ifndef NOMINMAX
    #define NOMINMAX
    #endif
    #include <winsock2.h>
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <sys/select.h>
    #include <sys/time.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
#endif
#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include "Utilidades.h"
#include "Excepciones.h"

// ---------------------------------------------------------------------------
// Tipos y constantes segun la plataforma
// ---------------------------------------------------------------------------
#ifdef _WIN32
    typedef SOCKET socket_t;
    static const socket_t SOCKET_NULO = INVALID_SOCKET;
    typedef int lon_dir_t;
    static const int BANDERA_ENVIO = 0;
#else
    typedef int socket_t;
    static const socket_t SOCKET_NULO = -1;
    typedef socklen_t lon_dir_t;
    #ifdef MSG_NOSIGNAL
    static const int BANDERA_ENVIO = MSG_NOSIGNAL;   // evita SIGPIPE si el cliente se cae
    #else
    static const int BANDERA_ENVIO = 0;
    #endif
#endif

// Codigo de error de la ultima operacion de red (errno / WSAGetLastError)
inline int ultimoErrorRed() {
#ifdef _WIN32
    return WSAGetLastError();
#else
    return errno;
#endif
}

// El error corresponde al vencimiento de SO_RCVTIMEO (EAGAIN/EWOULDBLOCK en POSIX,
// WSAETIMEDOUT/WSAEWOULDBLOCK en Windows)
inline bool esTimeout(int codigo) {
#ifdef _WIN32
    return codigo == WSAETIMEDOUT || codigo == WSAEWOULDBLOCK;
#else
    return codigo == EAGAIN || codigo == EWOULDBLOCK;
#endif
}

inline bool esInterrupcion(int codigo) {
#ifdef _WIN32
    return codigo == WSAEINTR;
#else
    return codigo == EINTR;
#endif
}

// Inicializa Winsock en Windows (WSAStartup / WSACleanup). En Linux no hace nada.
class InicializadorRed {
public:
    InicializadorRed() {
#ifdef _WIN32
        WSADATA datos;
        int r = WSAStartup(MAKEWORD(2, 2), &datos);
        if (r != 0) {
            throw ExcepcionSocket("Fallo en WSAStartup() (Winsock)", r);
        }
#endif
    }
    ~InicializadorRed() {
#ifdef _WIN32
        WSACleanup();
#endif
    }
    InicializadorRed(const InicializadorRed&) = delete;
    InicializadorRed& operator=(const InicializadorRed&) = delete;
};

// RAII: el descriptor se cierra solo, incluso si se lanza una excepcion
class DescriptorSocket {
private:
    socket_t fd;
public:
    DescriptorSocket() : fd(SOCKET_NULO) {}
    explicit DescriptorSocket(socket_t _fd) : fd(_fd) {}
    ~DescriptorSocket() { cerrar(); }

    DescriptorSocket(const DescriptorSocket&) = delete;
    DescriptorSocket& operator=(const DescriptorSocket&) = delete;

    DescriptorSocket(DescriptorSocket&& otro) noexcept : fd(otro.fd) { otro.fd = SOCKET_NULO; }
    DescriptorSocket& operator=(DescriptorSocket&& otro) noexcept {
        if (this != &otro) {
            cerrar();
            fd = otro.fd;
            otro.fd = SOCKET_NULO;
        }
        return *this;
    }

    socket_t get() const { return fd; }
    bool valido() const { return fd != SOCKET_NULO; }
    void cerrar() {
        if (fd != SOCKET_NULO) {
#ifdef _WIN32
            closesocket(fd);
#else
            ::close(fd);
#endif
            fd = SOCKET_NULO;
        }
    }
};

// Construye un sockaddr_in (IP + puerto en orden de red). Solo IPv4 con formato a.b.c.d
inline sockaddr_in crearDireccion(const std::string& ip, uint16_t puerto) {
    sockaddr_in dir;
    std::memset(&dir, 0, sizeof(dir));
    dir.sin_family = AF_INET;
    dir.sin_port = htons(puerto);
    if (ip.empty() || ip == "0.0.0.0") {
        dir.sin_addr.s_addr = htonl(INADDR_ANY);
        return dir;
    }
    unsigned a, b, c, d;
    char sobrante;
    if (std::sscanf(ip.c_str(), "%u.%u.%u.%u%c", &a, &b, &c, &d, &sobrante) != 4 ||
        a > 255 || b > 255 || c > 255 || d > 255) {
        throw ExcepcionSocket("Direccion IP invalida: " + ip);
    }
    uint32_t valor = (static_cast<uint32_t>(a) << 24) | (static_cast<uint32_t>(b) << 16) |
                     (static_cast<uint32_t>(c) << 8)  |  static_cast<uint32_t>(d);
    dir.sin_addr.s_addr = htonl(valor);
    return dir;
}

// Convierte la IP de un sockaddr_in a texto "a.b.c.d"
inline std::string ipATexto(const sockaddr_in& dir) {
    uint32_t v = ntohl(dir.sin_addr.s_addr);
    return aTexto((v >> 24) & 0xFF) + "." + aTexto((v >> 16) & 0xFF) + "." +
           aTexto((v >> 8) & 0xFF) + "." + aTexto(v & 0xFF);
}

// Timeout de recepcion (SO_RCVTIMEO): evita el bloqueo indefinido de recv/recvfrom.
// POSIX usa struct timeval; Windows usa un DWORD en milisegundos.
inline void configurarTimeoutRecepcion(socket_t fd, int segundos) {
#ifdef _WIN32
    DWORD ms = static_cast<DWORD>(segundos) * 1000;
    int r = setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO,
                       reinterpret_cast<const char*>(&ms), sizeof(ms));
#else
    timeval tv;
    tv.tv_sec = segundos;
    tv.tv_usec = 0;
    int r = setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
#endif
    if (r < 0) {
        throw ExcepcionSocket("Fallo en setsockopt(SO_RCVTIMEO)", ultimoErrorRed());
    }
}

// Espera hasta 'ms' milisegundos a que haya una conexion o datos pendientes.
// Devuelve >0 si esta listo, 0 si vencio el tiempo, <0 si hubo error.
inline int esperarLectura(socket_t fd, int ms) {
    fd_set conjunto;
    FD_ZERO(&conjunto);
    FD_SET(fd, &conjunto);
    timeval tv;
    tv.tv_sec = ms / 1000;
    tv.tv_usec = (ms % 1000) * 1000;
    return select(static_cast<int>(fd) + 1, &conjunto, nullptr, nullptr, &tv);
}

// TCP es un flujo de bytes: send() puede enviar menos de lo pedido -> se repite
inline void enviarTodo(socket_t fd, const std::string& datos) {
    size_t enviado = 0;
    while (enviado < datos.size()) {
        auto n = send(fd, datos.data() + enviado,
                      static_cast<int>(datos.size() - enviado), BANDERA_ENVIO);
        if (n < 0) {
            int codigo = ultimoErrorRed();
            if (esInterrupcion(codigo)) continue;
            throw ExcepcionSocket("Fallo en send()", codigo);
        }
        enviado += static_cast<size_t>(n);
    }
}

// Lee una linea terminada en '\n' (protocolo de trama sobre el flujo TCP)
inline std::string recibirLinea(socket_t fd) {
    std::string linea;
    char c;
    while (true) {
        auto n = recv(fd, &c, 1, 0);
        if (n == 0) {
            throw ExcepcionSocket("Conexion cerrada antes de recibir la respuesta");
        }
        if (n < 0) {
            int codigo = ultimoErrorRed();
            if (esInterrupcion(codigo)) continue;
            if (esTimeout(codigo)) {
                throw ExcepcionSocket("TIMEOUT esperando respuesta", codigo);
            }
            throw ExcepcionSocket("Fallo en recv()", codigo);
        }
        if (c == '\n') return linea;
        linea += c;
    }
}

#endif // SOCKET_UTIL_H
