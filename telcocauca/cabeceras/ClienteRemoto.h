
#ifndef CLIENTE_REMOTO_H
#define CLIENTE_REMOTO_H

#include <string>
#include <vector>
#include <stdexcept>
#include "ExclusionMutua.h"
#include "SocketUtil.h"
#include "Polimorfismo.h"
#include "Excepciones.h"

class ClienteRemoto {
private:
    std::string nombre;
    std::string ipServidor;
    uint16_t puerto;
public:
    ClienteRemoto(const std::string& _nombre, const std::string& _ipServidor, uint16_t _puerto)
        : nombre(_nombre), ipServidor(_ipServidor), puerto(_puerto) {}

   
    void ejecutar(std::vector<DispositivoComunicacion*> dispositivos, EstadisticasRed& est) {
        try {
            socket_t fd = socket(AF_INET, SOCK_STREAM, 0);
            if (fd == SOCKET_NULO) throw ExcepcionSocket("Fallo al crear socket", ultimoErrorRed());
            DescriptorSocket sock(fd);                       // RAII
            configurarTimeoutRecepcion(sock.get(), 3);

            sockaddr_in destino = crearDireccion(ipServidor, puerto);
            if (connect(sock.get(), reinterpret_cast<sockaddr*>(&destino),
                        static_cast<lon_dir_t>(sizeof(destino))) < 0) {
                throw ExcepcionSocket("Fallo en connect() hacia " + ipServidor + ":" +
                                      aTexto(puerto), ultimoErrorRed());
            }

            int enviadas = 0, confirmadas = 0;
            for (size_t i = 0; i < dispositivos.size(); ++i) {
                try {
                    validarDispositivo(dispositivos[i]);          // guarda nullptr
                    enviarTodo(sock.get(), dispositivos[i]->serializar() + "\n");
                    std::string respuesta = recibirLinea(sock.get());
                    ++enviadas;
                    if (respuesta.compare(0, 3, "ACK") == 0) ++confirmadas;
                    est.sumarExito();
                }
                catch (const std::invalid_argument& e) {
                    
                    est.sumarFallo();
                    registrarEventoSeguro(nombre + ": EXCEPCION capturada: " + e.what());
                }
            }
            registrarEventoSeguro(nombre + ": " + aTexto(enviadas) + " tramas enviadas, " +
                                  aTexto(confirmadas) + " confirmadas (ACK)");
        }
        catch (const ExcepcionSocket& e) {
            est.sumarFallo();
            registrarEventoSeguro(nombre + ": EXCEPCION de red: " + e.what());
        }
        catch (const std::exception& e) {
            est.sumarFallo();
            registrarEventoSeguro(nombre + ": EXCEPCION: " + e.what());
        }
        catch (...) {
            est.sumarFallo();
            registrarEventoSeguro(nombre + ": EXCEPCION desconocida");
        }
    }
};

#endif 
