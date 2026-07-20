/**
 * @file DFPlayerModule.cpp
 * @author J-Jair
 * @brief Implementación de los métodos de la clase DFPlayerModule para el control de audio.
 * @details Este archivo contiene toda la lógica de control del reproductor DFPlayer Mini, 
 *          incluyendo la gestión de puertos seriales, volumen, pistas y estados de reproducción.
 */

#include "DFPlayerModule.h"
#include "../Libraries/DFRobotDFPlayerMini.cpp"

/**
 * @brief Constructor de la clase DFPlayerModule.
 * @param rxPin Pin de recepción serial (RX).
 * @param txPin Pin de transmisión serial (TX).
 */
DFPlayerModule::DFPlayerModule(int rxPin, int txPin) {
    this->rxPin = rxPin;
    this->txPin = txPin;
}

/**
 * @brief Inicializa la comunicación serial por hardware con el módulo y configura el volumen inicial.
 */
void DFPlayerModule::begin() {
    mySerial = new HardwareSerial(1);
    mySerial->begin(9600, SERIAL_8N1, rxPin, txPin);

    delay(1000); // Pequeña pausa para estabilizar el módulo

    dfplayerDisponible = myDFPlayer.begin(*mySerial);

    if (dfplayerDisponible) {
        myDFPlayer.volume(currentVolume);
        myDFPlayer.play(1);
        isPlaying = true;
        lastSongPlayed = 1;
    }
}

/**
 * @brief Aumenta el volumen del reproductor en un nivel (máximo 30).
 */
void DFPlayerModule::subirVolumen() {
    if (!dfplayerDisponible) return;
    if (currentVolume < 30) {
        currentVolume++;
        myDFPlayer.volume(currentVolume);
    }
}

/**
 * @brief Disminuye el volumen del reproductor en un nivel (mínimo 0).
 */
void DFPlayerModule::bajarVolumen() {
    if (!dfplayerDisponible) return;
    if (currentVolume > 0) {
        currentVolume--;
        myDFPlayer.volume(currentVolume);
    }
}

/**
 * @brief Reanuda la reproducción de la canción actual.
 */
void DFPlayerModule::reanudarCancion() {
    if (!dfplayerDisponible) return;
    myDFPlayer.start();
    isPlaying = true;
}

/**
 * @brief Pausa la reproducción de la música actual.
 */
void DFPlayerModule::pausarCancion() {
    if (!dfplayerDisponible) return;
    myDFPlayer.pause();
    isPlaying = false;
}

/**
 * @brief Avanza a la siguiente pista de audio disponible.
 */
void DFPlayerModule::siguienteCancion() {
    if (!dfplayerDisponible) return;
    myDFPlayer.next();
    lastSongPlayed++;
    isPlaying = true;
}

/**
 * @brief Retrocede a la pista anterior de audio.
 */
void DFPlayerModule::anteriorCancion() {
    if (!dfplayerDisponible) return;
    myDFPlayer.previous();
    if (lastSongPlayed > 1) {
        lastSongPlayed--;
    }
    isPlaying = true;
}

/**
 * @brief Reproduce un número de pista (track) específico.
 * @param track Número de la canción a reproducir.
 */
void DFPlayerModule::play(int track) {
    if (!dfplayerDisponible) return;
    myDFPlayer.play(track);
    lastSongPlayed = track;
    isPlaying = true;
}

/**
 * @brief Detiene por completo la reproducción del audio.
 */
void DFPlayerModule::stop() {
    if (!dfplayerDisponible) return;
    myDFPlayer.stop();
    isPlaying = false;
}

/**
 * @brief Configura una pista específica para que se repita en bucle (loop).
 * @param track Número de la pista a repetir.
 */
void DFPlayerModule::loopTrack(int track) {
    if (!dfplayerDisponible) return;
    myDFPlayer.loop(track);
    lastSongPlayed = track;
    isPlaying = true;
}

/**
 * @brief Establece un nivel de volumen específico de forma directa.
 * @param vol Nivel de volumen deseado (entre 0 y 30).
 */
void DFPlayerModule::volume(int vol) {
    if (!dfplayerDisponible) return;
    currentVolume = constrain(vol, 0, 30);
    myDFPlayer.volume(currentVolume);
}

/**
 * @brief Consulta el estado actual de reproducción.
 * @return true si está reproduciendo, false si está pausado o detenido.
 */
bool DFPlayerModule::isPlayingState() {
    return isPlaying;
}

/**
 * @brief Reinicia la reproducción de la canción que se estaba reproduciendo actualmente.
 */
void DFPlayerModule::restartCurrentSong() {
    if (!dfplayerDisponible) return;
    myDFPlayer.play(lastSongPlayed);
    isPlaying = true;
}

/**
 * @brief Vuelve a reproducir obligatoriamente la primera pista (canción número 1).
 */
void DFPlayerModule::restartSong1() {
    if (!dfplayerDisponible) return;
    myDFPlayer.play(1);
    lastSongPlayed = 1;
    isPlaying = true;
}

// Métodos de control para botones físicos opcionales
/**
 * @brief Gestiona la acción del botón para pasar a la siguiente pista.
 */
void DFPlayerModule::handleButtonNext() {
    siguienteCancion();
}

/**
 * @brief Gestiona la acción del botón para retroceder a la pista anterior.
 */
void DFPlayerModule::handleButtonPrev() {
    anteriorCancion();
}

/**
 * @brief Controla el botón físico principal alternando entre pausa y reproducción.
 */
void DFPlayerModule::handleButtonFisico() {
    if (isPlaying) {
        pausarCancion();
    } else {
        reanudarCancion();
    }
}