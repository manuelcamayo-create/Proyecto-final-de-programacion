// ============================================================================
// MODULO 2 - Clase base SensorIoT (segunda rama de la herencia multiple)
// ============================================================================
#ifndef SENSOR_IOT_H
#define SENSOR_IOT_H

#include <string>

class SensorIoT {
protected:
    std::string tipoSensor;
    double valorLectura;
public:
    SensorIoT(const std::string& tipo, double lectura)
        : tipoSensor(tipo), valorLectura(lectura) {}

    std::string getTipoSensor() const { return tipoSensor; }
    double getValorLectura() const { return valorLectura; }
};

#endif // SENSOR_IOT_H
