// ============================================================================
// MODULO 2 - Clase base DispositivoRed (primera rama de la herencia multiple)
// ============================================================================
#ifndef DISPOSITIVO_RED_H
#define DISPOSITIVO_RED_H

#include <string>

class DispositivoRed {
protected:
    std::string direccionIP;
    std::string direccionMAC;
    bool estadoOperativo;
public:
    DispositivoRed(const std::string& ip, const std::string& mac, bool operativo = true)
        : direccionIP(ip), direccionMAC(mac), estadoOperativo(operativo) {}

    std::string getDireccionIP() const { return direccionIP; }
    std::string getDireccionMAC() const { return direccionMAC; }
    bool isOperativo() const { return estadoOperativo; }
    void setEstadoOperativo(bool operativo) { estadoOperativo = operativo; }
};

#endif // DISPOSITIVO_RED_H
