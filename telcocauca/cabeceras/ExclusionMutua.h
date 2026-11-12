
#ifndef EXCLUSION_MUTUA_H
#define EXCLUSION_MUTUA_H

#include <iostream>
#include <string>
#include "Utilidades.h"
#include <thread>
#include <mutex>
#include <map>


std::mutex mtx_consola;


const std::thread::id ID_HILO_PRINCIPAL = std::this_thread::get_id();

inline std::map<std::thread::id, int>& tablaHilos() {
    static std::map<std::thread::id, int> tabla;
    return tabla;
}


inline std::string etiquetaHilo(const std::thread::id& id) {
    if (id == ID_HILO_PRINCIPAL) return "MAIN";
    std::map<std::thread::id, int>& tabla = tablaHilos();
    auto it = tabla.find(id);
    if (it == tabla.end()) {
        it = tabla.insert(std::make_pair(id, static_cast<int>(tabla.size()) + 1)).first;
    }
    return "H" + aTexto(it->second);
}


inline void reiniciarNumeracionHilos() {
    std::lock_guard<std::mutex> lock(mtx_consola);
    tablaHilos().clear();
}


inline void imprimirSeguro(const std::string& texto) {
    std::lock_guard<std::mutex> lock(mtx_consola);
    std::cout << texto << std::endl;
}

inline void registrarEventoSeguro(const std::string& mensaje) {
   
    std::lock_guard<std::mutex> lock(mtx_consola);
    std::cout << "  [" << etiquetaHilo(std::this_thread::get_id()) << "] " << mensaje << std::endl;
}


class EventoIngenieria2027;
class EstadisticasRed;
inline std::string informeConsolidado(const EventoIngenieria2027& agenda,
                                      const EstadisticasRed& est);


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

#endif 
