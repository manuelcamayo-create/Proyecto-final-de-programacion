
#ifndef MULTIHILO_H
#define MULTIHILO_H

#include <vector>
#include <thread>
#include <functional>  
#include "Polimorfismo.h"
#include "ExclusionMutua.h"
#include "ManejoExcepciones.h"

class SupervisorRed {
public:
    void supervisar(DispositivoComunicacion* dev, EstadisticasRed& est) {
        tareaTransmisionHilo(dev, est);  
    }
};


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


inline void sincronizarHilos(std::vector<std::thread>& hilos) {
    for (auto& t : hilos) {
        if (t.joinable()) {
            t.join();
        }
    }
}

#endif 
