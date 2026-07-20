/**
 * @file DFPlayerModule.h
 * @author J-Jair
 * @brief Archivo de cabecera (Header) de la clase DFPlayerModule.
 * @details Define la estructura de la clase, las variables miembro y los prototipos de 
 *          las funciones para el manejo del reproductor DFPlayer Mini y sus entradas.
 */

#ifndef DFPlayerModule_h
#define DFPlayerModule_h

// Inclusión de librerías necesarias para el manejo del puerto serial y el módulo DFPlayer
#include <HardwareSerial.h>
#include "../Libraries/DFRobotDFPlayerMini.h"

/**
 * @class DFPlayerModule
 * @brief Clase principal para gestionar la reproducción de audio y control de volumen.
 */
class DFPlayerModule {
private:
    HardwareSerial* mySerial;          // Puntero para la comunicación serial por hardware con el ESP32
    DFRobotDFPlayerMini myDFPlayer;    // Instancia de la librería oficial de DFRobot
    bool dfplayerDisponible = false;   // Bandera para verificar si el módulo respondió correctamente

    int rxPin = 17;                    // Pin de recepción serial predeterminado
    int txPin = 16;                    // Pin de transmisión serial predeterminado
    int currentVolume = 20;            // Nivel de volumen actual por defecto (0-30)
    int lastSongPlayed = 1;            // Registro de la última pista reproducida
    bool isPlaying = false;            // Estado actual de reproducción (reproduciendo o pausado)

public:
    /**
     * @brief Constructor de la clase con valores de pines predeterminados.
     * @param rxPin Pin RX asignado para la comunicación serial.
     * @param txPin Pin TX asignado para la comunicación serial.
     */
    DFPlayerModule(int rxPin = 17, int txPin = 16);

    /**
     * @brief Inicializa el puerto serial y detecta el módulo DFPlayer Mini.
     */
    void begin();

    // Métodos de control de volumen
    void subirVolumen();
    void bajarVolumen();

    // Métodos de reproducción básicos
    void reanudarCancion();
    void pausarCancion();
    void siguienteCancion();
    void anteriorCancion();

    // Métodos avanzados de control de pistas y volumen directo
    void play(int track);
    void stop();
    void loopTrack(int track);
    void volume(int vol);

    // Métodos de consulta y reinicio de pistas
    bool isPlayingState();
    void restartCurrentSong();
    void restartSong1();

    // Métodos de gestión para botones físicos
    void handleButtonNext();
    void handleButtonPrev();
    void handleButtonFisico();

private:
    unsigned long lastPressTime = 0;   // Control de tiempo para evitar rebotes en botones de pistas
    unsigned long lastPressFisico = 0; // Control de tiempo para evitar rebotes en el botón principal
    bool isNextHeld = false;           // Estado lógico para pulsación sostenida del botón siguiente
    bool isPrevHeld = false;           // Estado lógico para pulsación sostenida del botón anterior
    bool isFisicoHeld = false;         // Estado lógico para pulsación sostenida del botón físico
};

#endif