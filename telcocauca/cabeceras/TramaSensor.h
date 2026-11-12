
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
    uint32_t idEstacion;       
    char     tipoDispositivo[16]; 
    float    temperatura;      
    float    humedad;        
    uint64_t timestamp;         
};
#pragma pack(pop)

static_assert(sizeof(TramaSensor) == 36, "TramaSensor debe medir 36 bytes (sin padding)");
static_assert(std::is_pod<TramaSensor>::value, "TramaSensor debe ser POD (sin punteros ni vtable)");

inline TramaSensor crearTrama(uint32_t id, const std::string& tipo,
                              float temperatura, float humedad, uint64_t timestamp) {
    TramaSensor t;
    std::memset(&t, 0, sizeof(t));
    t.idEstacion = id;
    std::strncpy(t.tipoDispositivo, tipo.c_str(), sizeof(t.tipoDispositivo) - 1); 
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

#endif 
