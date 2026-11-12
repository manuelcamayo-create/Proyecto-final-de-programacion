
#ifndef NODO_RED_INTELIGENTE_H
#define NODO_RED_INTELIGENTE_H

#include <iostream>
#include <string>
#include "DispositivoRed.h"
#include "SensorIoT.h"
#include "Excepciones.h"

class NodoRedInteligente : public DispositivoRed, public SensorIoT {
private:
    double cargaProcesamiento;   // porcentaje de CPU

    static void validarLectura(double v) {
        if (v < -40.0 || v > 125.0) {
            throw ExcepcionNodoRed("Lectura fuera de rango [-40, 125]: " + aTexto(v));
        }
    }
public:
    NodoRedInteligente(const std::string& ip, const std::string& mac,
                       const std::string& tipoSensor, double lectura, double carga)
        : DispositivoRed(ip, mac, true),
          SensorIoT(tipoSensor, lectura),
          cargaProcesamiento(carga) {
        if (ip.empty()) {
            throw ExcepcionNodoRed("Direccion IP vacia");
        }
        validarLectura(lectura);
    }

    void actualizarLectura(double nuevaLectura) {
        validarLectura(nuevaLectura);
        valorLectura = nuevaLectura;
    }

    double getCarga() const { return cargaProcesamiento; }

    // Suma la carga de procesamiento entre dos nodos
    double operator+(const NodoRedInteligente& otro) const {
        return cargaProcesamiento + otro.cargaProcesamiento;
    }

    // Impresion limpia en consola 
    friend std::ostream& operator<<(std::ostream& os, const NodoRedInteligente& n) {
        os << "  NodoInteligente [IP: " << n.direccionIP << " | MAC: " << n.direccionMAC
           << " | " << n.tipoSensor << ": " << n.valorLectura
           << " | Carga: " << n.cargaProcesamiento << "%]";
        return os;
    }
};

#endif 
