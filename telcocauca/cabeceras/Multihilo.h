// ============================================================================
// CRITERIO 2 (Multihilo & Exclusion Mutua) - MODULO 4
// std::vector<std::thread> + hilos sobre FUNCIONES MIEMBRO
//   std::thread t(&Clase::metodo, &instancia, std::ref(param))
// + paso por referencia con std::ref() + join()
// ============================================================================
#ifndef MULTIHILO_H
#define MULTIHILO_H

#include <vector>
#include <thread>
#include <functional>   // std::ref
#include "Polimorfismo.h"
#include "ExclusionMutua.h"
#include "ManejoExcepciones.h"

// Supervisor de la red: su metodo miembro es la rutina de cada hilo de supervision
class SupervisorRed {
public:
    void supervisar(DispositivoComunicacion* dev, EstadisticasRed& est) {
        tareaTransmisionHilo(dev, est);   // guarda nullptr + try/catch interno
    }
};

// Un hilo independiente por cada dispositivo del vector heterogeneo.
// - 'dev' se pasa POR VALOR (copia del puntero): la variable del ciclo muere en cada
//   vuelta y una referencia a ella quedaria colgando.
// - 'est' se pasa por REFERENCIA explicita con std::ref() porque el hilo la modifica.
inline std::vector<std::thread> lanzarTransmisiones(
        const std::vector<DispositivoComunicacion*>& red,
        SupervisorRed& supervisor,
        EstadisticasRed& est) {
    std::vector<std::thread> hilos;
    for (size_t i = 0; i < red.size(); ++i) {
        hilos.push_back(std::thread(&SupervisorRed::supervisar, &supervisor,
                                    red[i], std::ref(est)));
    }
    return hilos;
}

// Sincronizacion limpia: nunca se destruye un std::thread 'joinable'
// (si ocurriera, el runtime llamaria a std::terminate()).
inline void sincronizarHilos(std::vector<std::thread>& hilos) {
    for (auto& t : hilos) {
        if (t.joinable()) {
            t.join();
        }
    }
}

#endif // MULTIHILO_H
