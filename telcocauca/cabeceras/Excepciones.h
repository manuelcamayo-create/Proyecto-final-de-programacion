
#ifndef EXCEPCIONES_H
#define EXCEPCIONES_H

#include <stdexcept>
#include <string>
#include "Utilidades.h"

class ExcepcionNodoRed : public std::runtime_error {
public:
    explicit ExcepcionNodoRed(const std::string& msg)
        : std::runtime_error("[NodoRed] " + msg) {}
};


class ExcepcionSocket : public std::runtime_error {
private:
    int codigoErrno;
public:
    explicit ExcepcionSocket(const std::string& msg, int err = 0)
        : std::runtime_error("[Error] " + msg +
                             (err != 0 ? " [Errno: " + aTexto(err) + "]" : std::string())),
          codigoErrno(err) {}
    int getErrno() const { return codigoErrno; }
};


class ExcepcionRedUDP : public std::runtime_error {
private:
    int codigoErrno;
public:
    ExcepcionRedUDP(const std::string& msg, int err)
        : std::runtime_error(msg + " [Errno: " + aTexto(err) + "]"),
          codigoErrno(err) {}
    int getErrno() const { return codigoErrno; }
};

#endif 
