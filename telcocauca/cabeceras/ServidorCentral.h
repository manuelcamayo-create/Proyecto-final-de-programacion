
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
    DescriptorSocket escucha;              
    std::atomic<bool> detener;
    std::atomic<int> clientesAtendidos;
    RegistroEstados registro;

public:
    explicit ServidorCentral(uint16_t _puerto)
        : puerto(_puerto), detener(false), clientesAtendidos(0) {}

    void iniciar() {
        socket_t fd = socket(AF_INET, SOCK_STREAM, 0);
        if (fd == SOCKET_NULO) throw ExcepcionSocket("Fallo al crear socket", ultimoErrorRed());
        escucha = DescriptorSocket(fd);

#ifndef _WIN32
        
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

  
    void ejecutar() {
        std::vector<std::thread> hilosClientes;
        try {
            while (!detener) {
               
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
                    
                    registrarEventoSeguro(std::string("accept() fallido: ") +
                                          ExcepcionSocket("accept()", ultimoErrorRed()).what());
                    continue;
                }
                std::string ipCliente = ipATexto(cli) + ":" + aTexto(ntohs(cli.sin_port));

                
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

        for (auto& t : hilosClientes) {   
            if (t.joinable()) t.join();
        }
    }

    
    void atenderCliente(socket_t fdCliente, std::string ipCliente) {
        DescriptorSocket cliente(fdCliente); 
        try {
            configurarTimeoutRecepcion(cliente.get(), 1);
            std::string pendiente;
            char buffer[256];

            while (true) {
                auto n = recv(cliente.get(), buffer, static_cast<int>(sizeof(buffer)), 0);
                if (n == 0) break;                        
                if (n < 0) {
                    int codigo = ultimoErrorRed();
                    if (esInterrupcion(codigo)) continue;
                    if (esTimeout(codigo)) {
                        if (detener) break;                
                        continue;                         
                    }
                    throw ExcepcionSocket("Fallo en recv() con " + ipCliente, codigo);
                }
                pendiente.append(buffer, static_cast<size_t>(n));

                
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

#endif 
