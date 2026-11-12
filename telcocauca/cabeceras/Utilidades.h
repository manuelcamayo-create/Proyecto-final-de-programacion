// ============================================================================
// Utilidades de texto
// aTexto(): convierte numeros a std::string sin depender de std::to_string,
// que algunas versiones de MinGW no incluyen.
// ============================================================================
#ifndef UTILIDADES_H
#define UTILIDADES_H

#include <string>
#include <sstream>

template <typename T>
inline std::string aTexto(T valor) {
    std::ostringstream os;
    os << valor;
    return os.str();
}

#endif // UTILIDADES_H
