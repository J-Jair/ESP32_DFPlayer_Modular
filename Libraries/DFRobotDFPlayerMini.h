/*!
 * @file DFRobotDFPlayerMini.h
 * @brief Archivo de cabecera (Header) para la libreria de control del reproductor MP3 DFPlayer Mini.
 * @details Define las macros de configuracion, codigos de comandos, estados, errores y la estructura
 *          completa de la clase DFRobotDFPlayerMini con todos sus metodos publicos y privados.

 */

#include "Arduino.h"

#ifndef DFRobotDFPlayerMini_cpp
    #define DFRobotDFPlayerMini_cpp

// Definicion de perfiles de ecualizacion disponibles
#define DFPLAYER_EQ_NORMAL 0
#define DFPLAYER_EQ_POP 1
#define DFPLAYER_EQ_ROCK 2
#define DFPLAYER_EQ_JAZZ 3
#define DFPLAYER_EQ_CLASSIC 4
#define DFPLAYER_EQ_BASS 5

// Definicion de dispositivos de almacenamiento soportados
#define DFPLAYER_DEVICE_U_DISK 1
#define DFPLAYER_DEVICE_SD 2
#define DFPLAYER_DEVICE_AUX 3
#define DFPLAYER_DEVICE_SLEEP 4
#define DFPLAYER_DEVICE_FLASH 5

// Tamaños estandar en bytes para las tramas de comunicacion serial
#define DFPLAYER_RECEIVED_LENGTH 10
#define DFPLAYER_SEND_LENGTH 10

//#define _DEBUG

// Codigos de eventos, estados y tipos de mensajes reportados
#define TimeOut 0
#define WrongStack 1
#define DFPlayerCardInserted 2
#define DFPlayerCardRemoved 3
#define DFPlayerCardOnline 4
#define DFPlayerPlayFinished 5
#define DFPlayerError 6
#define DFPLAYER_DEVICE_U_DISK 1 // (Referencia interna duplicada del original)
#define DFPlayerUSBInserted 7
#define DFPlayerUSBRemoved 8
#define DFPlayerUSBOnline 9
#define DFPlayerCardUSBOnline 10
#define DFPlayerFeedBack 11

// Codigos de errores especificos devueltos por el modulo
#define Busy 1
#define Sleeping 2
#define SerialWrongStack 3
#define CheckSumNotMatch 4
#define FileIndexOut 5
#define FileMismatch 6
#define Advertise 7

// Indices de posicion de los bytes dentro de la estructura de la trama serial
#define Stack_Header 0
#define Stack_Version 1
#define Stack_Length 2
#define Stack_Command 3
#define Stack_ACK 4
#define Stack_Parameter 5
#define Stack_CheckSum 7
#define Stack_End 9

/**
 * @class DFRobotDFPlayerMini
 * @brief Clase principal para la gestion y control del modulo reproductor MP3 DFPlayer Mini.
 */
class DFRobotDFPlayerMini {
  Stream* _serial; ///< Puntero al flujo de comunicacion serial utilizado.
  
  unsigned long _timeOutTimer; ///< Temporizador interno para control de tiempos de espera.
  unsigned long _timeOutDuration = 500; ///< Duracion predeterminada del tiempo limite en milisegundos.
  
  uint8_t _received[DFPLAYER_RECEIVED_LENGTH]; ///< Arreglo de bytes para almacenar tramas entrantes.
  uint8_t _sending[DFPLAYER_SEND_LENGTH] = {0x7E, 0xFF, 06, 00, 01, 00, 00, 00, 00, 0xEF}; ///< Arreglo con la plantilla base para tramas de salida.
  
  uint8_t _receivedIndex=0; ///< Indice de seguimiento para la lectura de bytes entrantes.

  /** @brief Funciones internas para el envio y empaquetado de tramas de comandos. */
  void sendStack();
  void sendStack(uint8_t command);
  void sendStack(uint8_t command, uint16_t argument);
  void sendStack(uint8_t command, uint8_t argumentHigh, uint8_t argumentLow);

  /** @brief Funciones de configuracion para el protocolo de confirmacion de recepcion (ACK). */
  void enableACK();
  void disableACK();
  
  /** @brief Funciones auxiliares para conversion de datos numericos y arreglos de bytes. */
  void uint16ToArray(uint16_t value, uint8_t *array);
  uint16_t arrayToUint16(uint8_t *array);
  
  /** @brief Calcula la suma de verificacion de integridad de la trama. */
  uint16_t calculateCheckSum(uint8_t *buffer);

  /** @brief Metodos internos para analisis y validacion de tramas recibidas. */
  void parseStack();
  bool validateStack();
  
  uint8_t device = DFPLAYER_DEVICE_SD; ///< Dispositivo de almacenamiento activo por defecto.
  
  public:
  
  uint8_t _handleType; ///< Tipo de mensaje o evento actual procesado.
  uint8_t _handleCommand; ///< Codigo de comando actual procesado.
  uint16_t _handleParameter; ///< Parametro numerico asociado al mensaje actual.
  bool _isAvailable = false; ///< Bandera que indica si hay un mensaje nuevo disponible para leer.
  bool _isSending = false; ///< Bandera que indica si hay una transmision activa en curso.
  
  /**
   * @brief Maneja de forma interna la recepcion exitosa de un mensaje.
   * @param type Tipo de evento.
   * @param parameter Parametro opcional.
   * @return true
   */
  bool handleMessage(uint8_t type, uint16_t parameter = 0);

  /**
   * @brief Maneja de forma interna la ocurrencia de un error.
   * @param type Tipo de error.
   * @param parameter Parametro opcional.
   * @return false
   */
  bool handleError(uint8_t type, uint16_t parameter = 0);

  /**
   * @brief Lee el ultimo comando procesado.
   * @return uint8_t Codigo de comando.
   */
  uint8_t readCommand();
  
  /**
   * @brief Inicializa la comunicacion con el modulo.
   * @param stream Referencia al puerto serial.
   * @param isACK Activa o desactiva la confirmacion ACK.
   * @param doReset Ejecuta un reinicio del modulo al iniciar.
   * @return true Si la conexion es exitosa.
   */
  bool begin(Stream& stream, bool isACK = true, bool doReset = true);
  
  /**
   * @brief Espera hasta que haya datos disponibles en el puerto serial.
   * @param duration Tiempo maximo de espera opcional.
   * @return true Si hay datos listos.
   */
  bool waitAvailable(unsigned long duration = 0);
  
  /**
   * @brief Verifica si hay nuevos mensajes disponibles analizando el puerto serial.
   * @return true Si se recibio una trama valida completa.
   */
  bool available();
  
  /**
   * @brief Obtiene el tipo de evento actual y limpia la disponibilidad.
   * @return uint8_t Tipo de mensaje.
   */
  uint8_t readType();
  
  /**
   * @brief Obtiene el valor del parametro actual y limpia la disponibilidad.
   * @return uint16_t Valor numerico del parametro.
   */
  uint16_t read();
  
  /**
   * @brief Configura el tiempo limite de espera (timeout) para las respuestas.
   * @param timeOutDuration Duracion en milisegundos.
   */
  void setTimeOut(unsigned long timeOutDuration);
  
  /** @name Comandos de Reproduccion y Control Basico */
  /// @{
  void next();
  void previous();
  void play(int fileNumber=1);
  void volumeUp();
  void volumeDown();
  void volume(uint8_t volume);
  void EQ(uint8_t eq);
  void loop(int fileNumber);
  void outputDevice(uint8_t device);
  void sleep();
  void reset();
  void start();
  void pause();
  /// @}

  /** @name Comandos de Navegacion por Carpetas y Funciones Avanzadas */
  /// @{
  void playFolder(uint8_t folderNumber, uint8_t fileNumber);
  void outputSetting(bool enable, uint8_t gain);
  void enableLoopAll();
  void disableLoopAll();
  void playMp3Folder(int fileNumber);
  void advertise(int fileNumber);
  void playLargeFolder(uint8_t folderNumber, uint16_t fileNumber);
  void stopAdvertise();
  void stop();
  void loopFolder(int folderNumber);
  void randomAll();
  void enableLoop();
  void disableLoop();
  void enableDAC();
  void disableDAC();
  /// @}

  /** @name Metodos de Consulta de Estado y Parametros */
  /// @{
  int readState();
  int readVolume();
  int readEQ();
  int readFileCounts(uint8_t device);
  int readCurrentFileNumber(uint8_t device);
  int readFileCountsInFolder(int folderNumber);
  int readFileCounts();
  int readFolderCounts();
  int readCurrentFileNumber();
  /// @}
};

#endif