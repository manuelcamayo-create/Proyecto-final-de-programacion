// ============================================================================
// CRITERIO 4 (Resiliencia & Excepciones)
// Captura jerarquica try/catch DENTRO de cada hilo + guarda contra nullptr
// + transporte de excepciones al hilo principal con std::exception_ptr
// (Las excepciones personalizadas estan en Excepciones.h)
// ============================================================================
#ifndef MANEJO_EXCEPCIONES_H
#define MANEJO_EXCEPCIONES_H

#include <stdexcept>
#include <exception>
#include <string>
#include <vector>
#include "Excepciones.h"
#include "Polimorfismo.h"
#include "ExclusionMutua.h"

// Rutina que ejecuta cada hilo de transmision local
inline void tareaTransmisionHilo(DispositivoComunicacion* dev, EstadisticasRed& est) {
    try {
        validarDispositivo(dev);   // lanza std::invalid_argument si dev == nullptr
        dev->transmitir();         // invocacion polimorfica
        est.sumarExito();
        registrarEventoSeguro(dev->getId() + " transmitio (" + dev->getTipo() + ")");
    }
    // Captura jerarquica: de lo mas especifico a lo mas general
    catch (const std::invalid_argument& e) {
        est.sumarFallo();
        registrarEventoSeguro(std::string("EXCEPCION capturada en el hilo: ") + e.what());
    }
    catch (const std::exception& e) {
        est.sumarFallo();
        registrarEventoSeguro(std::string("EXCEPCION en el hilo: ") + e.what());
    }
    catch (...) {
        est.sumarFallo();
        registrarEventoSeguro("EXCEPCION desconocida en el hilo");
    }
}

// Ejecutada por el hilo principal: relanza las excepciones guardadas por otros hilos
inline void procesarErroresTransportados(const std::vector<std::exception_ptr>& errores) {
    for (size_t i = 0; i < errores.size(); ++i) {
        try {
            std::rethrow_exception(errores[i]);
        }
        catch (const std::exception& e) {
            registrarEventoSeguro(std::string("Excepcion recibida del hilo de agenda (exception_ptr): ") + e.what());
        }
    }
}

#endif // MANEJO_EXCEPCIONES_H
