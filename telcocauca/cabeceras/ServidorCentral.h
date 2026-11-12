// ============================================================================
// CRITERIO 3 (Sockets TCP/UDP & Red Distribuida) - SERVIDOR CENTRAL TCP CONCURRENTE
// socket() -> bind() -> listen() -> bucle accept(); un std::thread por cada cliente.
// Resiliencia: un error con un cliente se captura en SU hilo y no tumba al servidor.
// ============================================================================
#ifndef SERVIDOR_CENTRAL_H
#define SERVIDOR_CENTRAL_H

#include <atomic>
#include <thread>
#include <vector>
#include <string>
#include <mutex>
#include "ExclusionMutua.h"
#include "SocketUtil.h"
#include "Serializacion.h"
#include "Excepciones.h"

// Buffer compartido entre los hilos de los clientes (protegido con RAII)
class RegistroEstados {
private:
    std::vector<std::string> estados;
    mutable std::mutex mtx;
public:
    void agregar(const std::string& estado) {
        std::lock_guard<std::mutex> lock(mtx);
        estados.push_back(estado);
    }
    size_t cantidad() const {
        std::lock_guard<std::mutex> lock(mtx);
        return estados.size();
    }
};

class ServidorCentral {
private:
    uint16_t puerto;
    DescriptorSocket escucha;              // socket maestro (escucha)
    std::atomic<bool> detener;
    std::atomic<int> clientesAtendidos;
    RegistroEstados registro;

public:
    explicit ServidorCentral(uint16_t _puerto)
        : puerto(_puerto), detener(false), clientesAtendidos(0) {}

    // 1) socket()  2) bind()  3) listen()
    void iniciar() {
        socket_t fd = socket(AF_INET, SOCK_STREAM, 0);
        if (fd == SOCKET_NULO) throw ExcepcionSocket("Fallo al crear socket", ultimoErrorRed());
        escucha = DescriptorSocket(fd);

#ifndef _WIN32
        // En Linux permite reiniciar el servidor sin esperar a que el puerto se libere
        int opcion = 1;
        setsockopt(escucha.get(), SOL_SOCKET, SO_REUSEADDR, &opcion, sizeof(opcion));
#endif
        sockaddr_in dir = crearDireccion("0.0.0.0", puerto);
        if (bind(escucha.get(), reinterpret_cast<sockaddr*>(&dir),
                 static_cast<lon_dir_t>(sizeof(dir))) < 0) {
            throw ExcepcionSocket("Error en bind() del puerto " + aTexto(puerto), ultimoErrorRed());
        }
        if (listen(escucha.get(), 5) < 0) {
            throw ExcepcionSocket("Error en listen()", ultimoErrorRed());
        }
        registrarEventoSeguro("Servidor TCP escuchando en el puerto " + aTexto(puerto));
    }

    // 4) Bucle accept(): rutina del hilo del servidor
    void ejecutar() {
        std::vector<std::thread> hilosClientes;   // solo este hilo los crea y los une
        try {
            while (!detener) {
                // select() con 200 ms permite revisar la bandera 'detener' sin bloquear para siempre
                int r = esperarLectura(escucha.get(), 200);
                if (r < 0) {
                    int codigo = ultimoErrorRed();
                    if (esInterrupcion(codigo)) continue;
                    throw ExcepcionSocket("Error en select()", codigo);
                }
                if (r == 0) continue;

                sockaddr_in cli;
                lon_dir_t len = sizeof(cli);
                socket_t fdCliente = accept(escucha.get(), reinterpret_cast<sockaddr*>(&cli), &len);
                if (fdCliente == SOCKET_NULO) {
                    // Un accept fallido NO derriba al servidor
                    registrarEventoSeguro(std::string("accept() fallido: ") +
                                          ExcepcionSocket("accept()", ultimoErrorRed()).what());
                    continue;
                }
                std::string ipCliente = ipATexto(cli) + ":" + aTexto(ntohs(cli.sin_port));

                // Un hilo dedicado por cliente (funcion miembro: &Clase::metodo, this, args)
                hilosClientes.push_back(std::thread(&ServidorCentral::atenderCliente,
                                                    this, fdCliente, ipCliente));
            }
        }
        catch (const ExcepcionSocket& e) {
            registrarEventoSeguro(std::string("EXCEPCION de red en el servidor: ") + e.what());
        }
        catch (const std::exception& e) {
            registrarEventoSeguro(std::string("EXCEPCION en el servidor: ") + e.what());
        }
        catch (...) {
            registrarEventoSeguro("EXCEPCION desconocida en el servidor");
        }

        for (auto& t : hilosClientes) {   // sincronizacion obligatoria
            if (t.joinable()) t.join();
        }
    }

    // Rutina del hilo de cada cliente: recv() + deserializacion + ACK
    void atenderCliente(socket_t fdCliente, std::string ipCliente) {
        DescriptorSocket cliente(fdCliente);   // RAII: close() automatico
        try {
            configurarTimeoutRecepcion(cliente.get(), 1);
            std::string pendiente;
            char buffer[256];

            while (true) {
                auto n = recv(cliente.get(), buffer, static_cast<int>(sizeof(buffer)), 0);
                if (n == 0) break;                         // el cliente cerro la conexion
                if (n < 0) {
                    int codigo = ultimoErrorRed();
                    if (esInterrupcion(codigo)) continue;
                    if (esTimeout(codigo)) {
                        if (detener) break;                // servidor en apagado
                        continue;                          // cliente inactivo: seguir esperando
                    }
                    throw ExcepcionSocket("Fallo en recv() con " + ipCliente, codigo);
                }
                pendiente.append(buffer, static_cast<size_t>(n));

                // TCP no preserva fronteras de mensaje -> se separan tramas por '\n'
                size_t pos;
                while ((pos = pendiente.find('\n')) != std::string::npos) {
                    std::string linea = pendiente.substr(0, pos);
                    pendiente.erase(0, pos + 1);

                    std::string respuesta;
                    try {
                        EstadoDispositivo e = deserializarEstado(linea);
                        registro.agregar(linea);
                        registrarEventoSeguro("TCP RX " + e.id + " | " + e.tipo + " | " +
                                              e.ip + " | " + e.detalle);
                        respuesta = "ACK|" + e.id;
                    }
                    catch (const ExcepcionSocket& ex) {
                        registrarEventoSeguro(std::string("TCP RX rechazada: ") + ex.what());
                        respuesta = "NACK|trama invalida";
                    }
                    enviarTodo(cliente.get(), respuesta + "\n");
                }
            }
        }
        catch (const ExcepcionSocket& e) {
            registrarEventoSeguro("EXCEPCION con el cliente " + ipCliente + ": " + e.what());
        }
        catch (const std::exception& e) {
            registrarEventoSeguro("EXCEPCION con el cliente " + ipCliente + ": " + e.what());
        }
        catch (...) {
            registrarEventoSeguro("EXCEPCION desconocida con el cliente " + ipCliente);
        }
        ++clientesAtendidos;
    }

    void detenerServidor() { detener = true; }
    int getClientesAtendidos() const { return clientesAtendidos.load(); }
    size_t getTramasRecibidas() const { return registro.cantidad(); }
};

#endif // SERVIDOR_CENTRAL_H
