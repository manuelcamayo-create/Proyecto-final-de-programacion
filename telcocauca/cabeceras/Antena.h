
#ifndef ANTENA_H
#define ANTENA_H

#include <string>

class Tecnico;   // Declaracion adelantada: solo se usa un puntero

class Antena {
private:
    int idAntena;
    double frecuenciaMHz;
    std::string tecnologia;               // "4G" / "5G"
    Tecnico* responsableMantenimiento;    // Asociacion 1:1
public:
    Antena(int _id, double _frecuenciaMHz, const std::string& _tecnologia,
           Tecnico* _responsable = nullptr);
    ~Antena();

    int getIdAntena() const;
    double getFrecuenciaMHz() const;
    std::string getTecnologia() const;
    Tecnico* getResponsable() const;
    void asignarResponsable(Tecnico* _responsable);

    void mostrarInformacion() const;
};

#endif // ANTENA_H
