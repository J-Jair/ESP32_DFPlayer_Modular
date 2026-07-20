/**
 * @file DFPlayerMini.ino
 * @author J-Jair
 * @brief Archivo principal para control de reproductor de audio DFPlayer Mini con ESP32.
 * @details Este sketch gestiona la reproducción de música mediante un módulo externo 
 *          utilizando botones físicos para pasar canciones, retroceder y controlar la reproducción.
 */

// ================================================
// ARCHIVO PRINCIPAL .ino
// ================================================
// Usa el módulo DFPlayerModule (sin LED, solo música)
// Todo listo para copiar-pegar sin descargar nada extra
// ================================================

// Inclusión de los archivos de cabecera y código fuente del módulo personalizado ubicado en la ruta relativa
#include "../../Modules/DFPlayerModule.h"
#include "../../Modules/DFPlayerModule.cpp"

// ==================== PINES ====================
// Definición de los pines físicos conectados a los pulsadores de control
#define BUTTON_NEXT 23  // Pin para avanzar a la siguiente pista
#define BUTTON_PREV 22  // Pin para retroceder a la pista anterior
const int BotonFisico = 5; // Pin para reproducir o pausar la música

// ==================== INSTANCIA DEL MÓDULO ====================
// Creación del objeto reproductor indicando los pines RX (17) y TX (16) para la comunicación Serial con el ESP32
DFPlayerModule dfPlayer(17, 16);

/**
 * @brief Configuración inicial del ESP32.
 * Se ejecuta una sola vez al encender o reiniciar el microcontrolador.
 */
void setup() {
  // Inicialización de la comunicación serial para depuración a 115200 baudios
  Serial.begin(115200);
  
  // Inicialización del módulo reproductor DFPlayer
  dfPlayer.begin();

  // Configuración de los pines de los botones como entradas con resistencia pull-up interna activa
  pinMode(BotonFisico, INPUT_PULLUP);
  pinMode(BUTTON_NEXT, INPUT_PULLUP);
  pinMode(BUTTON_PREV, INPUT_PULLUP);
}

/**
 * @brief Bucle principal del programa.
 * Se ejecuta continuamente leyendo el estado de los controles de forma cíclica.
 */
void loop() {
  // Evalúa de forma constante si se presiona el botón para la siguiente pista
  dfPlayer.handleButtonNext();
  
  // Evalúa de forma constante si se presiona el botón para la pista anterior
  dfPlayer.handleButtonPrev();
  
  // Evalúa el estado del botón físico principal (Play/Pausa)
  dfPlayer.handleButtonFisico();

  // Pequeña pausa para estabilizar las lecturas y evitar rebotes rápidos en el bucle
  delay(250);
}