// ============================================================================
// CRITERIO 4 (Resiliencia & Excepciones) - JERARQUIA DE EXCEPCIONES PERSONALIZADAS
// Todas derivan de std::runtime_error (Modulos 2 y 5 del Proyecto Integrador)
// ============================================================================
#ifndef EXCEPCIONES_H
#define EXCEPCIONES_H

#include <stdexcept>
#include <string>
#include "Utilidades.h"

// Modulo 2: lecturas fuera de rango o fallas de IP en un nodo inteligente
class ExcepcionNodoRed : public std::runtime_error {
public:
    explicit ExcepcionNodoRed(const std::string& msg)
        : std::runtime_error("[NodoRed] " + msg) {}
};

// Modulo 5 (Unidad 9.1): fallos de la API de sockets TCP (socket, bind, listen, accept, connect, send, recv)
class ExcepcionSocket : public std::runtime_error {
private:
    int codigoErrno;
public:
    explicit ExcepcionSocket(const std::string& msg, int err = 0)
        : std::runtime_error("[Error] " + msg +
                             (err != 0 ? " [Errno: " + aTexto(err) + "]" : std::string())),
          codigoErrno(err) {}
    int getErrno() const { return codigoErrno; }
};

// Modulo 5 (Unidad 9.2): fallos y timeouts de sockets UDP (sendto / recvfrom)
class ExcepcionRedUDP : public std::runtime_error {
private:
    int codigoErrno;
public:
    ExcepcionRedUDP(const std::string& msg, int err)
        : std::runtime_error(msg + " [Errno: " + aTexto(err) + "]"),
          codigoErrno(err) {}
    int getErrno() const { return codigoErrno; }
};

#endif // EXCEPCIONES_H
