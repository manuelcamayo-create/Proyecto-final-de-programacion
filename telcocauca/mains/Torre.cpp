#include "Torre.h"
#include "Antena.h"  
#include <iostream>
#include <stdexcept>

Torre::Torre(int _id, const std::string& _ubicacion, double _alturaMetros)
    : idTorre(_id), ubicacion(_ubicacion), alturaMetros(_alturaMetros) {}

Torre::~Torre() {}

void Torre::agregarAntena(Antena* antena) {
    if (antena == nullptr) {
        throw std::invalid_argument("Antena nula rechazada por la torre");
    }
    listaAntenas.push_back(antena);
}

size_t Torre::getCantidadAntenas() const { return listaAntenas.size(); }

void Torre::mostrarInformacion() const {
    std::cout << "  Torre #" << idTorre << " | " << ubicacion << " | " << alturaMetros
              << " m | Antenas: " << listaAntenas.size() << std::endl;
    for (size_t i = 0; i < listaAntenas.size(); ++i) {
        listaAntenas[i]->mostrarInformacion();
    }
}
