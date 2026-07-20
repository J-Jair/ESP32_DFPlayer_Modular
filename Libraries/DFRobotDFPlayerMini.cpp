/*!
 * @file DFRobotDFPlayerMini.cpp
 * @brief Implementacion de la libreria para el control del reproductor MP3 DFPlayer Mini.
 * @details Este archivo contiene toda la logica de comunicacion serial, empaquetado de tramas, 
 *          envio de comandos y procesamiento de respuestas o errores devueltos por el modulo.
 
 */

#include "DFRobotDFPlayerMini.h"

/**
 * @brief Configura el tiempo limite de espera (timeout) para las respuestas del modulo.
 * @param timeOutDuration Duracion del tiempo limite en milisegundos.
 */
void DFRobotDFPlayerMini::setTimeOut(unsigned long timeOutDuration){
  _timeOutDuration = timeOutDuration;
}

/**
 * @brief Convierte un valor de 16 bits en un arreglo de dos bytes (alto y bajo).
 * @param value Valor numerico de 16 bits a convertir.
 * @param array Puntero al arreglo donde se almacenaran los bytes resultantes.
 */
void DFRobotDFPlayerMini::uint16ToArray(uint16_t value, uint8_t *array){
  *array = (uint8_t)(value>>8);
  *(array+1) = (uint8_t)(value);
}

/**
 * @brief Calcula la suma de verificacion (Checksum) para la trama de comandos a enviar.
 * @param buffer Puntero al arreglo de bytes que contiene la trama.
 * @return uint16_t Valor calculado de la suma de verificacion complementaria.
 */
uint16_t DFRobotDFPlayerMini::calculateCheckSum(uint8_t *buffer){
  uint16_t sum = 0;
  for (int i=Stack_Version; i<Stack_CheckSum; i++) {
    sum += buffer[i];
  }
  return -sum;
}

/**
 * @brief Envia la trama de datos estructurada a traves del puerto serial hacia el modulo MP3.
 */
void DFRobotDFPlayerMini::sendStack(){
  if (_sending[Stack_ACK]) {  // Si el modo de confirmacion (ACK) esta activo, espera hasta que termine la transmision anterior
    while (_isSending) {
      delay(0);
      waitAvailable();
    }
  }

#ifdef _DEBUG
  Serial.println();
  Serial.print(F("Enviando trama: "));
  for (int i=0; i<DFPLAYER_SEND_LENGTH; i++) {
    Serial.print(_sending[i], HEX);
    Serial.print(F(" "));
  }
  Serial.println();
#endif
  _serial->write(_sending, DFPLAYER_SEND_LENGTH);
  _timeOutTimer = millis();
  _isSending = _sending[Stack_ACK];
  
  if (!_sending[Stack_ACK]) { // Si el modo ACK esta desactivado, espera 10ms despues de cada transmision
    delay(10);
  }
}

/**
 * @brief Prepara y envia una trama de comandos simple sin argumentos adicionales.
 * @param command Codigo del comando a enviar.
 */
void DFRobotDFPlayerMini::sendStack(uint8_t command){
  sendStack(command, 0);
}

/**
 * @brief Prepara y envia una trama de comandos acompañada de un argumento numerico de 16 bits.
 * @param command Codigo del comando a enviar.
 * @param argument Valor numerico de 16 bits (parametro).
 */
void DFRobotDFPlayerMini::sendStack(uint8_t command, uint16_t argument){
  _sending[Stack_Command] = command;
  uint16ToArray(argument, _sending+Stack_Parameter);
  uint16ToArray(calculateCheckSum(_sending), _sending+Stack_CheckSum);
  sendStack();
}

/**
 * @brief Prepara y envia una trama dividiendo un argumento de 16 bits en dos bytes separados (alto y bajo).
 * @param command Codigo del comando a enviar.
 * @param argumentHigh Byte alto del parametro.
 * @param argumentLow Byte bajo del parametro.
 */
void DFRobotDFPlayerMini::sendStack(uint8_t command, uint8_t argumentHigh, uint8_t argumentLow){
  uint16_t buffer = argumentHigh;
  buffer <<= 8;
  sendStack(command, buffer | argumentLow);
}

/**
 * @brief Habilita el modo de confirmacion de recepcion (ACK) para los comandos enviados.
 */
void DFRobotDFPlayerMini::enableACK(){
  _sending[Stack_ACK] = 0x01;
}

/**
 * @brief Deshabilita el modo de confirmacion de recepcion (ACK) para los comandos enviados.
 */
void DFRobotDFPlayerMini::disableACK(){
  _sending[Stack_ACK] = 0x00;
}

/**
 * @brief Espera hasta que haya datos disponibles para leer desde el puerto serial o se agote el tiempo limite.
 * @param duration Tiempo maximo de espera opcional en milisegundos.
 * @return true Si hay datos disponibles a tiempo.
 * @return false Si ocurre un error de tiempo de espera agotado (TimeOut).
 */
bool DFRobotDFPlayerMini::waitAvailable(unsigned long duration){
  unsigned long timer = millis();
  if (!duration) {
    duration = _timeOutDuration;
  }
  while (!available()){
    if (millis() - timer > duration) {
      return handleError(TimeOut);
    }
    delay(0);
  }
  return true;
}

/**
 * @brief Inicializa la comunicacion con el modulo DFPlayer Mini, configurando el flujo serial y opcionalmente reiniciandolo.
 * @brief Referencia al flujo serial utilizado (Stream).
 * @brief Define si se activa la confirmacion ACK.
 * @brief Define si se envia una orden de reinicio al arrancar.
 * @return true Si el modulo responde correctamente y se conecta.
 * @return false Si falla la inicializacion.
 */
bool DFRobotDFPlayerMini::begin(Stream &stream, bool isACK, bool doReset){
  _serial = &stream;
  
  if (isACK) {
    enableACK();
  }
  else{
    disableACK();
  }
  
  if (doReset) {
    reset();
    waitAvailable(2000);
    delay(200);
  }
  else {
    // Asume el mismo estado inicial como si se hubiera reiniciado: tarjeta en linea
    _handleType = DFPlayerCardOnline;
  }

  return (readType() == DFPlayerCardOnline) || (readType() == DFPlayerUSBOnline) || !isACK;
}

/**
 * @brief Lee el tipo de evento o estado almacenado y limpia la bandera de disponibilidad.
 * @return uint8_t Tipo de mensaje procesado.
 */
uint8_t DFRobotDFPlayerMini::readType(){
  _isAvailable = false;
  return _handleType;
}

/**
 * @brief Lee el valor del parametro almacenado y limpia la bandera de disponibilidad.
 * @return uint16_t Valor numerico del parametro recibido.
 */
uint16_t DFRobotDFPlayerMini::read(){
  _isAvailable = false;
  return _handleParameter;
}

/**
 * @brief Almacena un mensaje recibido correctamente con su tipo y parametro asociado.
 * @param type Tipo de evento.
 * @param parameter Parametro numerico del evento.
 * @return true Indica que el mensaje esta disponible para ser leido.
 */
bool DFRobotDFPlayerMini::handleMessage(uint8_t type, uint16_t parameter){
  _receivedIndex = 0;
  _handleType = type;
  _handleParameter = parameter;
  _isAvailable = true;
  return _isAvailable;
}

/**
 * @brief Maneja de forma interna la ocurrencia de un error restableciendo el estado de transmision.
 * @param type Tipo de error reportado.
 * @param parameter Parametro opcional del error.
 * @return false Retorna siempre falso para indicar fallo en la operacion actual.
 */
bool DFRobotDFPlayerMini::handleError(uint8_t type, uint16_t parameter){
  handleMessage(type, parameter);
  _isSending = false;
  return false;
}

/**
 * @brief Lee el codigo de comando interno procesado y limpia la bandera de disponibilidad.
 * @return uint8_t Codigo del comando.
 */
uint8_t DFRobotDFPlayerMini::readCommand(){
  _isAvailable = false;
  return _handleCommand;
}

/**
 * @brief Analiza la trama de datos recibida desde el puerto serial y la clasifica segun su comando.
 */
void DFRobotDFPlayerMini::parseStack(){
  uint8_t handleCommand = *(_received + Stack_Command);
  if (handleCommand == 0x41) { // Caso especial para la retroalimentacion ACK (0x41), evita corromper los estados principales
    _isSending = false;
    return;
  }
  
  _handleCommand = handleCommand;
  _handleParameter = arrayToUint16(_received + Stack_Parameter);

  switch (_handleCommand) {
    case 0x3C:
    case 0x3D:
      handleMessage(DFPlayerPlayFinished, _handleParameter);
      break;
    case 0x3F:
      if (_handleParameter & 0x01) {
        handleMessage(DFPlayerUSBOnline, _handleParameter);
      }
      else if (_handleParameter & 0x02) {
        handleMessage(DFPlayerCardOnline, _handleParameter);
      }
      else if (_handleParameter & 0x03) {
        handleMessage(DFPlayerCardUSBOnline, _handleParameter);
      }
      break;
    case 0x3A:
      if (_handleParameter & 0x01) {
        handleMessage(DFPlayerUSBInserted, _handleParameter);
      }
      else if (_handleParameter & 0x02) {
        handleMessage(DFPlayerCardInserted, _handleParameter);
      }
      break;
    case 0x3B:
      if (_handleParameter & 0x01) {
        handleMessage(DFPlayerUSBRemoved, _handleParameter);
      }
      else if (_handleParameter & 0x02) {
        handleMessage(DFPlayerCardRemoved, _handleParameter);
      }
      break;
    case 0x40:
      handleMessage(DFPlayerError, _handleParameter);
      break;
    case 0x3E:
    case 0x42:
    case 0x43:
    case 0x44:
    case 0x45:
    case 0x46:
    case 0x47:
    case 0x48:
    case 0x49:
    case 0x4B:
    case 0x4C:
    case 0x4D:
    case 0x4E:
    case 0x4F:
      handleMessage(DFPlayerFeedBack, _handleParameter);
      break;
    default:
      handleError(WrongStack);
      break;
  }
}

/**
 * @brief Convierte dos bytes consecutivos de un arreglo en un valor numerico de 16 bits.
 * @param array Puntero al primer byte del arreglo.
 * @return uint16_t Valor combinado de 16 bits.
 */
uint16_t DFRobotDFPlayerMini::arrayToUint16(uint8_t *array){
  uint16_t value = *array;
  value <<= 8;
  value += *(array+1);
  return value;
}

/**
 * @brief Valida la integridad de la trama recibida comparando la suma de verificacion.
 * @return true Si el Checksum coincide correctamente.
 * @return false Si hay discrepancia en los datos.
 */
bool DFRobotDFPlayerMini::validateStack(){
  return calculateCheckSum(_received) == arrayToUint16(_received+Stack_CheckSum);
}

/**
 * @brief Lee y procesa byte a byte los datos entrantes desde el puerto serial para armar tramas validas.
 * @return true Si se ha recibido y procesado un mensaje completo con exito.
 * @return false Si aun no hay mensajes completos o se detecta un error de trama.
 */
bool DFRobotDFPlayerMini::available(){
  while (_serial->available()) {
    delay(0);
    if (_receivedIndex == 0) {
      _received[Stack_Header] = _serial->read();
#ifdef _DEBUG
      Serial.print(F("Recibido: "));
      Serial.print(_received[_receivedIndex], HEX);
      Serial.print(F(" "));
#endif
      if (_received[Stack_Header] == 0x7E) {
        _receivedIndex++;
      }
    }
    else {
      _received[_receivedIndex] = _serial->read();
#ifdef _DEBUG
      Serial.print(_received[_receivedIndex], HEX);
      Serial.print(F(" "));
#endif
      switch (_receivedIndex) {
        case Stack_Version:
          if (_received[_receivedIndex] != 0xFF) {
            return handleError(WrongStack);
          }
          break;
        case Stack_Length:
          if (_received[_receivedIndex] != 0x06) {
            return handleError(WrongStack);
          }
          break;
        case Stack_End:
#ifdef _DEBUG
          Serial.println();
#endif
          if (_received[_receivedIndex] != 0xEF) {
            return handleError(WrongStack);
          }
          else {
            if (validateStack()) {
              _receivedIndex = 0;
              parseStack();
              return _isAvailable;
            }
            else {
              return handleError(WrongStack);
            }
          }
          break;
        default:
          break;
      }
      _receivedIndex++;
    }
  }
  
  return _isAvailable;
}

/**
 * @brief Comando para reproducir la siguiente pista de audio.
 */
void DFRobotDFPlayerMini::next(){
  sendStack(0x01);
}

/**
 * @brief Comando para reproducir la pista de audio anterior.
 */
void DFRobotDFPlayerMini::previous(){
  sendStack(0x02);
}

/**
 * @brief Comando para reproducir una pista especifica basada en su numero de indice general.
 * @param fileNumber Numero de la pista a reproducir.
 */
void DFRobotDFPlayerMini::play(int fileNumber){
  sendStack(0x03, fileNumber);
}

/**
 * @brief Comando para incrementar el nivel de volumen en una unidad.
 */
void DFRobotDFPlayerMini::volumeUp(){
  sendStack(0x04);
}

/**
 * @brief Comando para disminuir el nivel de volumen en una unidad.
 */
void DFRobotDFPlayerMini::volumeDown(){
  sendStack(0x05);
}

/**
 * @brief Configura el nivel de volumen general del modulo.
 * @param volume Valor del volumen (Rango de 0 a 30).
 */
void DFRobotDFPlayerMini::volume(uint8_t volume){
  sendStack(0x06, volume);
}

/**
 * @brief Configura el perfil de ecualizacion del audio.
 * @param eq Tipo de ecualizacion deseado.
 */
void DFRobotDFPlayerMini::EQ(uint8_t eq) {
  sendStack(0x07, eq);
}

/**
 * @brief Configura la reproduccion en bucle continuo de una pista especifica.
 * @param fileNumber Numero de la pista a repetir.
 */
void DFRobotDFPlayerMini::loop(int fileNumber) {
  sendStack(0x08, fileNumber);
}

/**
 * @brief Selecciona el dispositivo de almacenamiento activo (por ejemplo, tarjeta SD o USB).
 * @param device Identificador del dispositivo de almacenamiento.
 */
void DFRobotDFPlayerMini::outputDevice(uint8_t device) {
  sendStack(0x09, device);
  delay(200);
}

/**
 * @brief Pone al modulo en modo de suspension (Sleep / Bajo consumo).
 */
void DFRobotDFPlayerMini::sleep(){
  sendStack(0x0A);
}

/**
 * @brief Reinicia por completo el modulo reproductor.
 */
void DFRobotDFPlayerMini::reset(){
  sendStack(0x0C);
}

/**
 * @brief Reanuda la reproduccion de audio actual (despues de una pausa).
 */
void DFRobotDFPlayerMini::start(){
  sendStack(0x0D);
}

/**
 * @brief Pausa temporalmente la reproduccion de audio actual.
 */
void DFRobotDFPlayerMini::pause(){
  sendStack(0x0E);
}

/**
 * @brief Reproduce un archivo especifico ubicado dentro de una carpeta especifica (ej. carpeta 01, archivo 001).
 * @param folderNumber Numero de la carpeta (1 a 99).
 * @param fileNumber Numero del archivo (1 a 255).
 */
void DFRobotDFPlayerMini::playFolder(uint8_t folderNumber, uint8_t fileNumber){
  sendStack(0x0F, folderNumber, fileNumber);
}

/**
 * @brief Configura los parametros de salida de audio y ganancia del amplificador interno.
 * @param enable Habilita o deshabilita la salida.
 * @param gain Nivel de ganancia deseado.
 */
void DFRobotDFPlayerMini::outputSetting(bool enable, uint8_t gain){
  sendStack(0x10, enable, gain);
}

/**
 * @brief Activa el bucle de reproduccion general para todos los archivos disponibles.
 */
void DFRobotDFPlayerMini::enableLoopAll(){
  sendStack(0x11, 0x01);
}

/**
 * @brief Desactiva el bucle de reproduccion general para todos los archivos.
 */
void DFRobotDFPlayerMini::disableLoopAll(){
  sendStack(0x11, 0x00);
}

/**
 * @brief Reproduce un archivo especifico ubicado en la carpeta raiz especial llamada "MP3".
 * @param fileNumber Numero del archivo de audio.
 */
void DFRobotDFPlayerMini::playMp3Folder(int fileNumber){
  sendStack(0x12, fileNumber);
}

/**
 * @brief Reproduce un archivo de anuncio publicitario o pista superpuesta desde la carpeta "ADVERT".
 * @param fileNumber Numero del archivo de anuncio.
 */
void DFRobotDFPlayerMini::advertise(int fileNumber){
  sendStack(0x13, fileNumber);
}

/**
 * @brief Reproduce un archivo en carpetas numeradas grandes (soporta mayor cantidad de carpetas y pistas).
 * @param folderNumber Numero de carpeta.
 * @param fileNumber Numero de archivo.
 */
void DFRobotDFPlayerMini::playLargeFolder(uint8_t folderNumber, uint16_t fileNumber){
  sendStack(0x14, (((uint16_t)folderNumber) << 12) | fileNumber);
}

/**
 * @brief Detiene la reproduccion del archivo de anuncio actual y regresa a la pista principal.
 */
void DFRobotDFPlayerMini::stopAdvertise(){
  sendStack(0x15);
}

/**
 * @brief Detiene por completo la reproduccion de audio.
 */
void DFRobotDFPlayerMini::stop(){
  sendStack(0x16);
}

/**
 * @brief Configura la reproduccion en bucle de todos los archivos contenidos dentro de una carpeta especifica.
 * @param folderNumber Numero de la carpeta a repetir.
 */
void DFRobotDFPlayerMini::loopFolder(int folderNumber){
  sendStack(0x17, folderNumber);
}

/**
 * @brief Reproduce de forma aleatoria (Shuffle) todas las pistas disponibles en el dispositivo.
 */
void DFRobotDFPlayerMini::randomAll(){
  sendStack(0x18);
}

/**
 * @brief Habilita el modo de bucle continuo para la pista actual.
 */
void DFRobotDFPlayerMini::enableLoop(){
  sendStack(0x19, 0x00);
}

/**
 * @brief Deshabilita el modo de bucle continuo para la pista actual.
 */
void DFRobotDFPlayerMini::disableLoop(){
  sendStack(0x19, 0x01);
}

/**
 * @brief Habilita el conversor digital-analógico (DAC) interno del modulo.
 */
void DFRobotDFPlayerMini::enableDAC(){
  sendStack(0x1A, 0x00);
}

/**
 * @brief Deshabilita el conversor digital-analógico (DAC) interno del modulo.
 */
void DFRobotDFPlayerMini::disableDAC(){
  sendStack(0x1A, 0x01);
}

/**
 * @brief Consulta el estado actual de reproduccion del modulo.
 * @return int Estado actual devuelto o -1 si ocurre un error de lectura.
 */
int DFRobotDFPlayerMini::readState(){
  sendStack(0x42);
  if (waitAvailable()) {
    if (readType() == DFPlayerFeedBack) {
      return read();
    }
    else {
      return -1;
    }
  }
  else {
    return -1;
  }
}

/**
 * @brief Consulta el nivel de volumen configurado actualmente en el modulo.
 * @return int Nivel de volumen actual o -1 si falla la lectura.
 */
int DFRobotDFPlayerMini::readVolume(){
  sendStack(0x43);
  if (waitAvailable()) {
    return read();
  }
  else {
    return -1;
  }
}

/**
 * @brief Consulta el modo de ecualizacion activo en el modulo.
 * @return int Modo de EQ actual o -1 si falla la lectura.
 */
int DFRobotDFPlayerMini::readEQ(){
  sendStack(0x44);
  if (waitAvailable()) {
    if (readType() == DFPlayerFeedBack) {
      return read();
    }
    else {
      return -1;
    }
  }
  else {
    return -1;
  }
}

/**
 * @brief Consulta la cantidad total de archivos de audio disponibles en un dispositivo de almacenamiento especifico.
 * @param device Dispositivo de almacenamiento a consultar.
 * @return int Conteo total de archivos o -1 en caso de error.
 */
int DFRobotDFPlayerMini::readFileCounts(uint8_t device){
  switch (device) {
    case DFPLAYER_DEVICE_U_DISK:
      sendStack(0x47);
      break;
    case DFPLAYER_DEVICE_SD:
      sendStack(0x48);
      break;
    case DFPLAYER_DEVICE_FLASH:
      sendStack(0x49);
      break;
    default:
      break;
  }
  
  if (waitAvailable()) {
    if (readType() == DFPlayerFeedBack) {
      return read();
    }
    else {
      return -1;
    }
  }
  else {
    return -1;
  }
}

/**
 * @brief Consulta el numero de pista que se esta reproduciendo actualmente en un dispositivo especifico.
 * @param device Dispositivo de almacenamiento a consultar.
 * @return int Numero de archivo actual o -1 en caso de error.
 */
int DFRobotDFPlayerMini::readCurrentFileNumber(uint8_t device){
  switch (device) {
    case DFPLAYER_DEVICE_U_DISK:
      sendStack(0x4B);
      break;
    case DFPLAYER_DEVICE_SD:
      sendStack(0x4C);
      break;
    case DFPLAYER_DEVICE_FLASH:
      sendStack(0x4D);
      break;
    default:
      break;
  }
  if (waitAvailable()) {
    if (readType() == DFPlayerFeedBack) {
      return read();
    }
    else {
      return -1;
    }
  }
  else {
    return -1;
  }
}

/**
 * @brief Consulta la cantidad de archivos ubicados dentro de una carpeta especifica de la tarjeta SD.
 * @param folderNumber Numero de la carpeta a consultar.
 * @return int Cantidad de archivos en la carpeta o -1 si ocurre un error.
 */
int DFRobotDFPlayerMini::readFileCountsInFolder(int folderNumber){
  sendStack(0x4E, folderNumber);
  if (waitAvailable()) {
    if (readType() == DFPlayerFeedBack) {
      return read();
    }
    else {
      return -1;
    }
  }
  else {
    return -1;
  }
}

/**
 * @brief Consulta el numero total de carpetas disponibles en el almacenamiento.
 * @return int Conteo total de carpetas o -1 en caso de error.
 */
int DFRobotDFPlayerMini::readFolderCounts(){
  sendStack(0x4F);
  if (waitAvailable()) {
    if (readType() == DFPlayerFeedBack) {
      return read();
    }
    else {
      return -1;
    }
  }
  else {
    return -1;
  }
}

/**
 * @brief Funcion auxiliar para consultar la cantidad total de archivos en la tarjeta SD por defecto.
 * @return int Conteo total de archivos o -1 en caso de error.
 */
int DFRobotDFPlayerMini::readFileCounts(){
  return readFileCounts(DFPLAYER_DEVICE_SD);
}

/**
 * @brief Funcion auxiliar para consultar el numero de pista actual en la tarjeta SD por defecto.
 * @return int Numero de pista actual o -1 en caso de error.
 */
int DFRobotDFPlayerMini::readCurrentFileNumber(){
  return readCurrentFileNumber(DFPLAYER_DEVICE_SD);
}