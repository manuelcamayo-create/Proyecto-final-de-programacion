// ============================================================================
// CRITERIO 3 - Deserializacion de tramas de texto (TCP)
// Formato: tipo|id|ip|detalle   (lo produce ITransmisible::serializar())
// ============================================================================
#ifndef SERIALIZACION_H
#define SERIALIZACION_H

#include <string>
#include <vector>
#include "Excepciones.h"

struct EstadoDispositivo {
    std::string tipo;
    std::string id;
    std::string ip;
    std::string detalle;
};

inline EstadoDispositivo deserializarEstado(const std::string& linea) {
    std::vector<std::string> campos;
    std::string actual;
    for (size_t i = 0; i < linea.size(); ++i) {
        if (linea[i] == '|') {
            campos.push_back(actual);
            actual.clear();
        } else {
            actual += linea[i];
        }
    }
    campos.push_back(actual);

    if (campos.size() != 4) {
        throw ExcepcionSocket("Trama mal formada (se esperaban 4 campos): " + linea);
    }
    EstadoDispositivo e = { campos[0], campos[1], campos[2], campos[3] };
    return e;
}

#endif // SERIALIZACION_H
