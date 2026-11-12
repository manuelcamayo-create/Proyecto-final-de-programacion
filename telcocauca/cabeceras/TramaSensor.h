// ============================================================================
// CRITERIO 3 - Serializacion binaria con #pragma pack (Unidad 9.2)
// Estructura POD empaquetada a 1 byte (sin padding) para enviarla tal cual por UDP.
// ============================================================================
#ifndef TRAMA_SENSOR_H
#define TRAMA_SENSOR_H

#include <cstdint>
#include <cstring>
#include <string>
#include <sstream>
#include <iomanip>
#include <type_traits>

#pragma pack(push, 1)
struct TramaSensor {
    uint32_t idEstacion;        // 4 bytes
    char     tipoDispositivo[16]; // 16 bytes
    float    temperatura;       // 4 bytes
    float    humedad;           // 4 bytes
    uint64_t timestamp;         // 8 bytes
};
#pragma pack(pop)

// Garantias en tiempo de compilacion
static_assert(sizeof(TramaSensor) == 36, "TramaSensor debe medir 36 bytes (sin padding)");
static_assert(std::is_pod<TramaSensor>::value, "TramaSensor debe ser POD (sin punteros ni vtable)");

inline TramaSensor crearTrama(uint32_t id, const std::string& tipo,
                              float temperatura, float humedad, uint64_t timestamp) {
    TramaSensor t;
    std::memset(&t, 0, sizeof(t));
    t.idEstacion = id;
    std::strncpy(t.tipoDispositivo, tipo.c_str(), sizeof(t.tipoDispositivo) - 1); // deja el '\0' final
    t.temperatura = temperatura;
    t.humedad = humedad;
    t.timestamp = timestamp;
    return t;
}

inline std::string describirTrama(const TramaSensor& t) {
    size_t largo = 0;
    while (largo < sizeof(t.tipoDispositivo) && t.tipoDispositivo[largo] != '\0') ++largo;
    std::string tipo(t.tipoDispositivo, largo);
    std::ostringstream os;
    os << "Estacion " << t.idEstacion << " | " << tipo << " | "
       << std::fixed << std::setprecision(1) << t.temperatura << "C | " << t.humedad << "%";
    return os.str();
}

#endif // TRAMA_SENSOR_H
