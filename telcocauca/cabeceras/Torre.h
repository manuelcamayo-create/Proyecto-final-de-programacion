// ============================================================================
// MODULO 1 - Entidad Torre. Asociacion 1:N con Antena (std::vector<Antena*>)
// La declaracion adelantada "class Antena;" evita la inclusion ciclica de cabeceras.
// ============================================================================
#ifndef TORRE_H
#define TORRE_H

#include <string>
#include <vector>

class Antena;   // Declaracion adelantada (resuelve dependencias circulares)

class Torre {
private:
    int idTorre;
    std::string ubicacion;
    double alturaMetros;
    std::vector<Antena*> listaAntenas;   // Asociacion 1:N (la torre NO es duena de las antenas)
public:
    Torre(int _id, const std::string& _ubicacion, double _alturaMetros);
    ~Torre();

    void agregarAntena(Antena* antena);  // lanza std::invalid_argument si es nullptr
    size_t getCantidadAntenas() const;
    void mostrarInformacion() const;
};

#endif // TORRE_H
