
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

#endif 
