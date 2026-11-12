// ============================================================================
// CRITERIO 1 (Arquitectura POO & Polimorfismo) - MODULO 3
// Interfaz pura + clase base abstracta + destructores virtuales + override
// + guarda contra nullptr para vectores polimorficos
// ============================================================================
#ifndef POLIMORFISMO_H
#define POLIMORFISMO_H

#include <string>
#include "Utilidades.h"
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <thread>
#include <chrono>

// Interfaz pura (solo metodos virtuales puros, SIN atributos de datos).
// Establece el contrato de transmision y de serializacion.
class ITransmisible {
public:
    virtual ~ITransmisible() = default;
    virtual void transmitir() = 0;
    virtual std::string serializar() const = 0;   // formato: tipo|id|ip|detalle
};

// Clase base abstracta (contrato polimorfico)
class DispositivoComunicacion : public ITransmisible {
protected:
    std::string id;
    std::string tipo;
    std::string ip;

    // Apoyo para serializar: arma la trama de texto comun a todos los dispositivos
    std::string serializarBase(const std::string& detalle) const {
        return tipo + "|" + id + "|" + ip + "|" + detalle;
    }
public:
    DispositivoComunicacion(std::string _id, std::string _tipo, std::string _ip)
        : id(_id), tipo(_tipo), ip(_ip) {}

    // Destructor virtual: delete sobre un puntero base destruye la subclase
    virtual ~DispositivoComunicacion() = default;

    // Metodo virtual puro: la clase sigue siendo abstracta
    virtual void transmitir() = 0;

    std::string getId() const { return id; }
    std::string getTipo() const { return tipo; }
    std::string getIp() const { return ip; }
};

// Subclases concretas: implementan los metodos puros con 'override'
class RouterWiFi : public DispositivoComunicacion {
private:
    int canalesActivos;
public:
    RouterWiFi(std::string _id, std::string _ip, int _canales)
        : DispositivoComunicacion(_id, "Router WiFi 6", _ip), canalesActivos(_canales) {}

    void transmitir() override {
        // Transmision sobre la banda de 5 GHz
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
    }
    std::string serializar() const override {
        return serializarBase("canales=" + aTexto(canalesActivos));
    }
};

class GatewayLoRa : public DispositivoComunicacion {
private:
    float frecuenciaMHz;
public:
    GatewayLoRa(std::string _id, std::string _ip, float _freq)
        : DispositivoComunicacion(_id, "Gateway LoRaWAN", _ip), frecuenciaMHz(_freq) {}

    void transmitir() override {
        // Transmision de larga distancia y bajo consumo
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    std::string serializar() const override {
        std::ostringstream os;
        os << "frecuencia=" << std::fixed << std::setprecision(1) << frecuenciaMHz << "MHz";
        return serializarBase(os.str());
    }
};

class SwitchFibra : public DispositivoComunicacion {
private:
    int puertos;
public:
    SwitchFibra(std::string _id, std::string _ip, int _puertos)
        : DispositivoComunicacion(_id, "Switch de Fibra Optica", _ip), puertos(_puertos) {}

    void transmitir() override {
        // Conmutacion sobre enlaces de fibra optica
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    std::string serializar() const override {
        return serializarBase("puertos=" + aTexto(puertos));
    }
};

// Guarda defensiva para vectores polimorficos: se invoca ANTES de usar el puntero
inline void validarDispositivo(const DispositivoComunicacion* dev) {
    if (dev == nullptr) {
        throw std::invalid_argument("Puntero nulo en el vector de dispositivos");
    }
}

#endif // POLIMORFISMO_H
