#include "Antena.h"
#include "Tecnico.h"
#include <iostream>

Antena::Antena(int _id, double _frecuenciaMHz, const std::string& _tecnologia, Tecnico* _responsable)
    : idAntena(_id), frecuenciaMHz(_frecuenciaMHz), tecnologia(_tecnologia),
      responsableMantenimiento(_responsable) {}

Antena::~Antena() {}

int Antena::getIdAntena() const { return idAntena; }
double Antena::getFrecuenciaMHz() const { return frecuenciaMHz; }
std::string Antena::getTecnologia() const { return tecnologia; }
Tecnico* Antena::getResponsable() const { return responsableMantenimiento; }
void Antena::asignarResponsable(Tecnico* _responsable) { responsableMantenimiento = _responsable; }

void Antena::mostrarInformacion() const {
    std::cout << "    Antena #" << idAntena << " | " << tecnologia << " | "
              << frecuenciaMHz << " MHz | Responsable: ";
    if (responsableMantenimiento != nullptr) {   // guarda defensiva
        std::cout << responsableMantenimiento->getNombre();
    } else {
        std::cout << "(sin asignar)";
    }
    std::cout << std::endl;
}
