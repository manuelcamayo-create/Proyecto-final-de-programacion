
#ifndef RED_UDP_H
#define RED_UDP_H

#include <atomic>
#include <mutex>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include "ExclusionMutua.h"
#include "SocketUtil.h"
#include "TramaSensor.h"
#include "Excepciones.h"

class ReceptorUDP {
private:
    uint16_t puerto;
    int timeoutSegundos;
    DescriptorSocket sock;
    std::vector<TramaSensor> recibidas;
    mutable std::mutex mtx;               
    std::atomic<bool> detener;

    void guardar(const TramaSensor& t) {
        std::lock_guard<std::mutex> lock(mtx);
        recibidas.push_back(t);
    }
public:
    ReceptorUDP(uint16_t _puerto, int _timeoutSegundos = 3)
        : puerto(_puerto), timeoutSegundos(_timeoutSegundos), detener(false) {}

    
    void abrir() {
        socket_t fd = socket(AF_INET, SOCK_DGRAM, 0);
        if (fd == SOCKET_NULO) throw ExcepcionSocket("Fallo al crear socket UDP", ultimoErrorRed());
        sock = DescriptorSocket(fd);

        sockaddr_in dir = crearDireccion("0.0.0.0", puerto);
        if (bind(sock.get(), reinterpret_cast<sockaddr*>(&dir),
                 static_cast<lon_dir_t>(sizeof(dir))) < 0) {
            throw ExcepcionSocket("Error en bind() UDP del puerto " + aTexto(puerto),
                                  ultimoErrorRed());
        }
        configurarTimeoutRecepcion(sock.get(), timeoutSegundos);
        registrarEventoSeguro("Receptor UDP en el puerto " + aTexto(puerto) +
                              " (timeout " + aTexto(timeoutSegundos) + " s)");
    }

    
    TramaSensor recibirUna() {
        TramaSensor trama;
        std::memset(&trama, 0, sizeof(trama));
        sockaddr_in remitente;
        lon_dir_t len = sizeof(remitente);

        auto bytes = recvfrom(sock.get(), reinterpret_cast<char*>(&trama),
                              static_cast<int>(sizeof(trama)), 0,
                              reinterpret_cast<sockaddr*>(&remitente), &len);
        if (bytes < 0) {
            int codigo = ultimoErrorRed();
            if (esTimeout(codigo)) {
                throw ExcepcionRedUDP("TIMEOUT: No se recibio datagrama", codigo);
            }
            throw ExcepcionRedUDP("ERROR CATASTROFICO UDP", codigo);
        }
        if (static_cast<size_t>(bytes) != sizeof(TramaSensor)) {
            throw ExcepcionRedUDP("Datagrama de tamano invalido (" + aTexto(bytes) + " bytes)", 0);
        }
        trama.tipoDispositivo[sizeof(trama.tipoDispositivo) - 1] = '\0';  // terminador defensivo
        return trama;
    }

    void ejecutar(int esperadas) {
        int recibidasOk = 0;
        try {
            while (recibidasOk < esperadas) {
                TramaSensor t = recibirUna();
                guardar(t);
                ++recibidasOk;
                registrarEventoSeguro("UDP RX " + describirTrama(t));
            }
            registrarEventoSeguro("UDP completo: " + aTexto(recibidasOk) + " tramas");
        }
        catch (const ExcepcionRedUDP& e) {
            registrarEventoSeguro(std::string("EXCEPCION ExcepcionRedUDP: ") + e.what() +
                                  " (" + aTexto(recibidasOk) + "/" + aTexto(esperadas) + " tramas)");
        }
        catch (const ExcepcionSocket& e) {
            registrarEventoSeguro(std::string("EXCEPCION de socket UDP: ") + e.what());
        }
        catch (const std::exception& e) {
            registrarEventoSeguro(std::string("EXCEPCION en el receptor UDP: ") + e.what());
        }
        catch (...) {
            registrarEventoSeguro("EXCEPCION desconocida en el receptor UDP");
        }
    }

    
    void escucharContinuo() {
        try {
            while (!detener) {
                try {
                    TramaSensor t = recibirUna();
                    guardar(t);
                    registrarEventoSeguro("UDP RX " + describirTrama(t));
                }
                catch (const ExcepcionRedUDP& e) {
                    if (!esTimeout(e.getErrno())) throw;
                   
                }
            }
        }
        catch (const ExcepcionRedUDP& e) {
            registrarEventoSeguro(std::string("EXCEPCION ExcepcionRedUDP: ") + e.what());
        }
        catch (const std::exception& e) {
            registrarEventoSeguro(std::string("EXCEPCION en el receptor UDP: ") + e.what());
        }
        catch (...) {
            registrarEventoSeguro("EXCEPCION desconocida en el receptor UDP");
        }
    }

    void detenerRecepcion() { detener = true; }
    size_t cantidadRecibidas() const {
        std::lock_guard<std::mutex> lock(mtx);
        return recibidas.size();
    }
};

class EmisorUDP {
private:
    DescriptorSocket sock;
    sockaddr_in destino;
public:
    EmisorUDP(const std::string& ip, uint16_t puerto) : destino() {
        socket_t fd = socket(AF_INET, SOCK_DGRAM, 0);
        if (fd == SOCKET_NULO) throw ExcepcionSocket("Fallo al crear socket UDP", ultimoErrorRed());
        sock = DescriptorSocket(fd);
        destino = crearDireccion(ip, puerto);
    }

    
    void enviar(const TramaSensor& t) {
        auto n = sendto(sock.get(), reinterpret_cast<const char*>(&t),
                        static_cast<int>(sizeof(t)), 0,
                        reinterpret_cast<const sockaddr*>(&destino),
                        static_cast<lon_dir_t>(sizeof(destino)));
        if (n < 0) {
            throw ExcepcionRedUDP("Fallo en sendto()", ultimoErrorRed());
        }
    }

    
    void ejecutar(std::vector<TramaSensor> tramas, int pausaMs) {
        try {
            for (size_t i = 0; i < tramas.size(); ++i) {
                enviar(tramas[i]);
                std::this_thread::sleep_for(std::chrono::milliseconds(pausaMs));
            }
            registrarEventoSeguro("UDP TX: " + aTexto(tramas.size()) + " tramas enviadas");
        }
        catch (const ExcepcionRedUDP& e) {
            registrarEventoSeguro(std::string("EXCEPCION ExcepcionRedUDP en el emisor: ") + e.what());
        }
        catch (const std::exception& e) {
            registrarEventoSeguro(std::string("EXCEPCION en el emisor UDP: ") + e.what());
        }
        catch (...) {
            registrarEventoSeguro("EXCEPCION desconocida en el emisor UDP");
        }
    }
};

#endif 
