
#ifndef TORRE_H
#define TORRE_H

#include <string>
#include <vector>

class Antena;   

class Torre {
private:
    int idTorre;
    std::string ubicacion;
    double alturaMetros;
    std::vector<Antena*> listaAntenas;   
public:
    Torre(int _id, const std::string& _ubicacion, double _alturaMetros);
    ~Torre();

    void agregarAntena(Antena* antena);  
    size_t getCantidadAntenas() const;
    void mostrarInformacion() const;
};

#endif 
