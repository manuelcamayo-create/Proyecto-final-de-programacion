// ============================================================================
// CRITERIO 2 / MODULO 4 - EVENTOS DE INGENIERIA 2027
// Clase EventoIngenieria2027: registra, procesa y despliega la agenda en un
// HILO DEDICADO sincronizado con std::mutex.
// ============================================================================
#ifndef EVENTO_INGENIERIA_2027_H
#define EVENTO_INGENIERIA_2027_H

#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <algorithm>
#include <stdexcept>
#include <exception>
#include "ExclusionMutua.h"

class EventoIngenieria2027 {
public:
    struct Evento {
        std::string nombre;  // Nombre del evento
        int         anio;    // Anio del evento
    };

private:
    std::vector<Evento> agenda;
    mutable std::mutex mtx_agenda;            // Protege el vector 'agenda'
    std::vector<std::exception_ptr> errores;  // Excepciones para el hilo principal
                                              // (las escribe el hilo de agenda; main
                                              //  las lee DESPUES del join)
    friend std::string informeConsolidado(const EventoIngenieria2027&,
                                          const EstadisticasRed&);
public:
    // Registro seguro: valida el dato y luego bloquea con lock_guard (RAII)
    void registrar(const Evento& ev) {
        if (ev.nombre.empty()) {
            throw std::invalid_argument("Evento sin nombre, no se registra");
        }
        std::lock_guard<std::mutex> lock(mtx_agenda);
        agenda.push_back(ev);
    }

    // Copia de la agenda tomada bajo el mutex
    std::vector<Evento> copiarAgenda() const {
        std::lock_guard<std::mutex> lock(mtx_agenda);
        return agenda;
    }

    const std::vector<std::exception_ptr>& getErrores() const { return errores; }

    // ---- RUTINA DEL HILO DEDICADO: registra, procesa y despliega ----
    // Se lanza como funcion miembro: std::thread(&Clase::metodo, &objeto, argumentos)
    void procesarAgenda(std::vector<Evento> eventos, EstadisticasRed& est) {
        try {
            // 1. Registro: cada evento se aisla con su propio try/catch
            for (size_t i = 0; i < eventos.size(); ++i) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                try {
                    registrar(eventos[i]);
                    registrarEventoSeguro("Evento 2027 registrado: " + eventos[i].nombre);
                }
                catch (const std::exception& e) {
                    // Captura interna + transporte al hilo principal
                    errores.push_back(std::current_exception());
                    registrarEventoSeguro(std::string("EXCEPCION al registrar evento: ") + e.what());
                }
            }

            // 2. Procesamiento: lectura segura y orden por anio y nombre
            std::vector<Evento> lista = copiarAgenda();
            if (lista.empty()) {
                throw std::runtime_error("La agenda de eventos 2027 esta vacia");
            }
            std::sort(lista.begin(), lista.end(),
                      [](const Evento& a, const Evento& b) {
                          if (a.anio != b.anio) return a.anio < b.anio;
                          return a.nombre < b.nombre;
                      });

            // 3. Despliegue: una sola linea, en una sola llamada protegida
            std::string linea = "AGENDA 2027 (" + aTexto(lista.size()) + "): ";
            for (size_t i = 0; i < lista.size(); ++i) {
                if (i > 0) linea += " | ";
                linea += lista[i].nombre + " " + aTexto(lista[i].anio);
            }
            registrarEventoSeguro(linea);

            // 4. Informe que consulta DOS recursos compartidos a la vez
            registrarEventoSeguro(informeConsolidado(*this, est));
        }
        catch (const std::exception& e) {
            registrarEventoSeguro(std::string("EXCEPCION en el hilo de agenda: ") + e.what());
        }
        catch (...) {
            registrarEventoSeguro("EXCEPCION desconocida en el hilo de agenda");
        }
    }
};

// Unidad 8.2 - Prevencion de deadlocks: se necesitan DOS mutex a la vez
// (agenda y estadisticas). std::lock los adquiere sin riesgo de interbloqueo
// y std::adopt_lock entrega su liberacion a lock_guard (RAII).
inline std::string informeConsolidado(const EventoIngenieria2027& ag, const EstadisticasRed& est) {
    std::lock(ag.mtx_agenda, est.mtx);
    std::lock_guard<std::mutex> lockAgenda(ag.mtx_agenda, std::adopt_lock);
    std::lock_guard<std::mutex> lockEst(est.mtx, std::adopt_lock);
    return "INFORME (std::lock con 2 mutex): eventos=" + aTexto(ag.agenda.size()) +
           " | transmisiones ok=" + aTexto(est.exitosas) + " fallos=" + aTexto(est.fallidas);
}

#endif // EVENTO_INGENIERIA_2027_H
