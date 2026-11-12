// ============================================================================
// MODULO 2 - NodoRedInteligente: HERENCIA MULTIPLE (DispositivoRed + SensorIoT)
// - Invocacion explicita de los constructores base en la lista de inicializacion
// - operator+ (suma de carga de procesamiento) y operator<< (friend)
// - ExcepcionNodoRed ante lecturas fuera de rango o IP invalida
// ============================================================================
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

    // Impresion limpia en consola (funcion amiga)
    friend std::ostream& operator<<(std::ostream& os, const NodoRedInteligente& n) {
        os << "  NodoInteligente [IP: " << n.direccionIP << " | MAC: " << n.direccionMAC
           << " | " << n.tipoSensor << ": " << n.valorLectura
           << " | Carga: " << n.cargaProcesamiento << "%]";
        return os;
    }
};

#endif // NODO_RED_INTELIGENTE_H
