
#include "SocketUtil.h"

#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <functional>   
#include <csignal>

#include "Tecnico.h"
#include "Antena.h"
#include "Torre.h"
#include "NodoRedInteligente.h"
#include "Excepciones.h"
#include "Polimorfismo.h"
#include "ExclusionMutua.h"
#include "ManejoExcepciones.h"
#include "Multihilo.h"
#include "EventoIngenieria2027.h"
#include "TramaSensor.h"
#include "ServidorCentral.h"
#include "ClienteRemoto.h"
#include "RedUDP.h"

static const uint16_t PUERTO_TCP = 8080;
static const uint16_t PUERTO_UDP = 8081;

static void titulo(const std::string& texto) {
    imprimirSeguro("\n== " + texto + " ==");
}


static void subtitulo(const std::string& texto) {
    reiniciarNumeracionHilos();
    imprimirSeguro(" - " + texto);
}

static std::vector<DispositivoComunicacion*> crearRedLocal() {
    std::vector<DispositivoComunicacion*> red;
    red.push_back(new RouterWiFi("RTR-01", "192.168.1.1", 12));
    red.push_back(new GatewayLoRa("GW-868", "192.168.1.20", 868.1f));
    red.push_back(new SwitchFibra("SW-FO-01", "192.168.1.30", 24));
    red.push_back(nullptr);
    return red;
}

static void liberarRed(std::vector<DispositivoComunicacion*>& red) {
    for (DispositivoComunicacion* dev : red) {
        delete dev;  
    }
    red.clear();
}


static std::vector<EventoIngenieria2027::Evento> crearEventos2027() {
    EventoIngenieria2027::Evento icassp   = { "IEEE ICASSP", 2027 };
    EventoIngenieria2027::Evento icc      = { "IEEE ICC", 2027 };
    EventoIngenieria2027::Evento wfiot    = { "IEEE WF-IoT", 2027 };
    EventoIngenieria2027::Evento invalido = { "", 2027 };

    std::vector<EventoIngenieria2027::Evento> eventos;
    eventos.push_back(icassp);
    eventos.push_back(icc);
    eventos.push_back(wfiot);
    eventos.push_back(invalido);
    return eventos;
}


static void bloque1POO(const std::vector<DispositivoComunicacion*>& red) {
    titulo("1. POO: dominio, herencia y polimorfismo");

   
    subtitulo("Modulo 1: Tecnico, Antena y Torre");
    Tecnico tecnico(1, "Carlos Munoz", "Radiofrecuencia");
    Antena antena1(10, 3500.0, "5G", &tecnico);
    Antena antena2(11, 1800.0, "4G", &tecnico);
    Antena antena3(12, 2600.0, "4G");
    Torre torre(100, "Cerro Munchique, Cauca", 45.0);
    torre.agregarAntena(&antena1);
    torre.agregarAntena(&antena2);
    torre.agregarAntena(&antena3);
    tecnico.mostrarInformacion();
    torre.mostrarInformacion();
    try {
        torre.agregarAntena(nullptr);
    }
    catch (const std::invalid_argument& e) {
        std::cout << "  Excepcion (guarda nullptr): " << e.what() << std::endl;
    }
    subtitulo("Modulo 2: herencia multiple y excepcion personalizada");
    try {
        NodoRedInteligente nodo1("192.168.1.10", "AA:BB:CC:00:00:01", "Temperatura", 24.5, 35.0);
        NodoRedInteligente nodo2("192.168.1.11", "AA:BB:CC:00:00:02", "Humedad", 60.0, 20.0);
        std::cout << nodo1 << std::endl << nodo2 << std::endl;
        std::cout << "  Carga combinada (nodo1 + nodo2): " << (nodo1 + nodo2) << "%" << std::endl;
        nodo1.actualizarLectura(200.0);   
    }
    catch (const ExcepcionNodoRed& e) {
        std::cout << "  Excepcion: " << e.what() << std::endl;
    }

   
    subtitulo("Modulo 3: vector polimorfico y trama empaquetada");
    std::string ids;
    for (size_t i = 0; i < red.size(); ++i) {
        if (i > 0) ids += ", ";
        ids += (red[i] != nullptr) ? red[i]->getId() : std::string("nullptr");
    }
    std::cout << "  Dispositivos en el Heap: " << ids << std::endl;
    std::cout << "  sizeof(TramaSensor) = " << sizeof(TramaSensor) << " bytes (sin padding)" << std::endl;
}

static void bloque2Hilos(std::vector<DispositivoComunicacion*>& red,
                         EstadisticasRed& estadisticas, EventoIngenieria2027& agenda) {
    titulo("2. Hilos, mutex y eventos 2027");

    subtitulo("Un hilo por dispositivo (cada uno termina cuando acaba su transmision)");
    SupervisorRed supervisor;
    std::vector<std::thread> hilos = lanzarTransmisiones(red, supervisor, estadisticas);
    sincronizarHilos(hilos);

    
    subtitulo("Hilo dedicado: agenda de eventos 2027");
    std::thread hiloAgenda(&EventoIngenieria2027::procesarAgenda,
                           &agenda, crearEventos2027(), std::ref(estadisticas));
    if (hiloAgenda.joinable()) {
        hiloAgenda.join();
    }

    
    procesarErroresTransportados(agenda.getErrores());
}


static void ejecutarNodoRemoto(ClienteRemoto& nodo, const std::string& descripcion,
                               const std::vector<DispositivoComunicacion*>& dispositivos,
                               EstadisticasRed& estadisticas) {
    subtitulo(descripcion);
    std::thread hiloNodo(&ClienteRemoto::ejecutar, &nodo, dispositivos, std::ref(estadisticas));
    hiloNodo.join();
}

static void bloque3TCP(const std::vector<DispositivoComunicacion*>& red, EstadisticasRed& estadisticas) {
    titulo("3. Red TCP: servidor central y nodos remotos");


    subtitulo("Servidor Central (un hilo por cada cliente aceptado)");
    ServidorCentral servidor(PUERTO_TCP);
    try {
        servidor.iniciar();
    }
    catch (const ExcepcionSocket& e) {
        std::cout << "  No se pudo iniciar el servidor: " << e.what() << std::endl;
        return;
    }
    std::thread hiloServidor(&ServidorCentral::ejecutar, &servidor);

    
    ClienteRemoto estacionA("Estacion-A", "127.0.0.1", PUERTO_TCP);
    ClienteRemoto estacionB("Estacion-B", "127.0.0.1", PUERTO_TCP);
    ClienteRemoto estacionCaida("Estacion-Caida", "127.0.0.1", 9);     
    std::vector<DispositivoComunicacion*> subconjunto(red.begin(), red.begin() + 2);

    ejecutarNodoRemoto(estacionA, "Estacion-A envia 4 dispositivos (uno es nullptr)", red, estadisticas);
    ejecutarNodoRemoto(estacionB, "Estacion-B envia 2 dispositivos", subconjunto, estadisticas);
    ejecutarNodoRemoto(estacionCaida, "Estacion-Caida intenta conectar a un puerto sin servidor", subconjunto, estadisticas);

    servidor.detenerServidor();
    if (hiloServidor.joinable()) {
        hiloServidor.join();
    }
    subtitulo("Resultado del servidor");
    registrarEventoSeguro(aTexto(servidor.getTramasRecibidas()) + " tramas recibidas de " +
                          aTexto(servidor.getClientesAtendidos()) + " clientes");
}

static void bloque4UDP() {
    titulo("4. Red UDP: telemetria con timeout de 3 s");

    try {
        subtitulo("Receptor UDP (socket + bind + recvfrom con timeout) y emisor (sendto)");
        ReceptorUDP receptor(PUERTO_UDP, 3);
        receptor.abrir();
        EmisorUDP emisor("127.0.0.1", PUERTO_UDP);

      
        std::vector<TramaSensor> tramas;
        tramas.push_back(crearTrama(101, "Gateway-LoRa", 24.5f, 60.0f, 1700000000ULL));
        tramas.push_back(crearTrama(502, "Sensor-WiFi", 28.4f, 55.2f, 1700005000ULL));

        std::thread hiloReceptor(&ReceptorUDP::ejecutar, &receptor, 3); 
        std::thread hiloEmisor(&EmisorUDP::ejecutar, &emisor, tramas, 300);
        hiloEmisor.join();
        hiloReceptor.join();                                           
    }
    catch (const ExcepcionSocket& e) {
        std::cout << "  No se pudo iniciar UDP: " << e.what() << std::endl;
    }
}


int main() {
#ifndef _WIN32
    std::signal(SIGPIPE, SIG_IGN);
#endif

    try {
        InicializadorRed inicializadorRed;   

        std::cout << "=== TELCOCAUCA: monitoreo distribuido de telecomunicaciones e IoT ===" << std::endl;

        std::vector<DispositivoComunicacion*> red = crearRedLocal();
        EstadisticasRed estadisticas;
        EventoIngenieria2027 agenda2027;

        try {
            bloque1POO(red);
            bloque2Hilos(red, estadisticas, agenda2027);
            bloque3TCP(red, estadisticas);
            bloque4UDP();
        }
        catch (...) {
            liberarRed(red);   
            throw;
        }

        titulo("Resumen");
        std::cout << "  " << estadisticas.resumen() << std::endl;
        liberarRed(red);
        std::cout << "  Memoria dinamica liberada (sin fugas)" << std::endl;
        std::cout << "  Sistema finalizado correctamente." << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "[Error Catastrofico]: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
