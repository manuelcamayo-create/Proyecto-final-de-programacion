// ============================================================================
// CRITERIO 3 (1.0 Pt) - EXCLUSION MUTUA (RAII)
// std::mutex + std::lock_guard sobre la consola y sobre variables compartidas
// NOTA: este archivo debe incluirse desde UN solo .cpp (main.cpp), porque
//       define la variable global mtx_consola.
// ============================================================================
#ifndef EXCLUSION_MUTUA_H
#define EXCLUSION_MUTUA_H

#include <iostream>
#include <string>
#include "Utilidades.h"
#include <thread>
#include <mutex>
#include <map>

// ---- Proteccion de la salida estandar compartida ----
std::mutex mtx_consola;

// Identificador del hilo principal (se inicializa antes de main, en el hilo principal)
const std::thread::id ID_HILO_PRINCIPAL = std::this_thread::get_id();

inline std::map<std::thread::id, int>& tablaHilos() {
    static std::map<std::thread::id, int> tabla;
    return tabla;
}

// Etiqueta del hilo: MAIN para el principal; H1, H2, H3... para los demas.
// (solo se llama con mtx_consola bloqueado)
inline std::string etiquetaHilo(const std::thread::id& id) {
    if (id == ID_HILO_PRINCIPAL) return "MAIN";
    std::map<std::thread::id, int>& tabla = tablaHilos();
    auto it = tabla.find(id);
    if (it == tabla.end()) {
        it = tabla.insert(std::make_pair(id, static_cast<int>(tabla.size()) + 1)).first;
    }
    return "H" + aTexto(it->second);
}

// Reinicia la numeracion H1, H2... (se llama entre pasos, cuando no hay hilos activos)
inline void reiniciarNumeracionHilos() {
    std::lock_guard<std::mutex> lock(mtx_consola);
    tablaHilos().clear();
}

// Imprime un texto completo de forma segura (titulos y subtitulos)
inline void imprimirSeguro(const std::string& texto) {
    std::lock_guard<std::mutex> lock(mtx_consola);
    std::cout << texto << std::endl;
}

inline void registrarEventoSeguro(const std::string& mensaje) {
    // RAII: bloquea al crearse, desbloquea automaticamente al salir (incluso con excepcion)
    std::lock_guard<std::mutex> lock(mtx_consola);
    std::cout << "  [" << etiquetaHilo(std::this_thread::get_id()) << "] " << mensaje << std::endl;
}

// Declaraciones adelantadas: informeConsolidado (Unidad 8.2) usa dos mutex a la vez
class EventoIngenieria2027;
class EstadisticasRed;
inline std::string informeConsolidado(const EventoIngenieria2027& agenda,
                                      const EstadisticasRed& est);

// ---- Variables compartidas entre los hilos de transmision (seccion critica) ----
class EstadisticasRed {
private:
    int exitosas;
    int fallidas;
    mutable std::mutex mtx; // Protege los contadores

    friend std::string informeConsolidado(const EventoIngenieria2027&,
                                          const EstadisticasRed&);
public:
    EstadisticasRed() : exitosas(0), fallidas(0) {}

    void sumarExito() {
        std::lock_guard<std::mutex> lock(mtx);
        ++exitosas;
    }
    void sumarFallo() {
        std::lock_guard<std::mutex> lock(mtx);
        ++fallidas;
    }
    std::string resumen() const {
        std::lock_guard<std::mutex> lock(mtx);
        return "Transmisiones: ok=" + aTexto(exitosas) + " fallos=" + aTexto(fallidas);
    }
};

#endif // EXCLUSION_MUTUA_H
