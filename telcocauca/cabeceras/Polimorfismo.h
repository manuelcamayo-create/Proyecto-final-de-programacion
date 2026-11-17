
#ifndef POLIMORFISMO_H
#define POLIMORFISMO_H

#include <string>
#include "Utilidades.h"
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <thread>
#include <chrono>


class ITransmisible {                                 //abtraccion 
public:
    virtual ~ITransmisible() = default;
    virtual void transmitir() = 0;
    virtual std::string serializar() const = 0;    //abtraccion 
};


class DispositivoComunicacion : public ITransmisible {
protected:
    std::string id;
    std::string tipo;
    std::string ip;

    
    std::string serializarBase(const std::string& detalle) const {
        return tipo + "|" + id + "|" + ip + "|" + detalle;
    }
public:
    DispositivoComunicacion(std::string _id, std::string _tipo, std::string _ip)
        : id(_id), tipo(_tipo), ip(_ip) {}

    virtual ~DispositivoComunicacion() = default; //el destrucctor virtual

    
    virtual void transmitir() = 0;   //puro, p

    std::string getId() const { return id; }
    std::string getTipo() const { return tipo; }
    std::string getIp() const { return ip; }
};

class RouterWiFi : public DispositivoComunicacion {           //herencia 
private:
    int canalesActivos;
public:
    RouterWiFi(std::string _id, std::string _ip, int _canales)
        : DispositivoComunicacion(_id, "Router WiFi 6", _ip), canalesActivos(_canales) {}

    void transmitir() override { //polimorfismo, sobre escritura 
        
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

    void transmitir() override { //polimorfismo
       
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

    void transmitir() override { //polimorfismo
     
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    std::string serializar() const override {
        return serializarBase("puertos=" + aTexto(puertos));
    }
};                    //herencia 


inline void validarDispositivo(const DispositivoComunicacion* dev) { //la validacion del puntero nulo
    if (dev == nullptr) {
        throw std::invalid_argument("Puntero nulo en el vector de dispositivos");
    }
}

#endif
