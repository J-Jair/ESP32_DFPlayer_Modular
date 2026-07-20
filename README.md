# ESP32_DFPlayer_Modular


## 1. Descripción General del Sistema
Este proyecto implementa un reproductor de audio digital basado en el microcontrolador **ESP32** y el módulo MP3 **DFPlayer Mini**. La arquitectura del software está modularizada en clases orientadas a objetos (`DFPlayerModule` y `DFRobotDFPlayerMini`), lo que encapsula toda la lógica de comunicación serial por hardware, tramas de comandos, control de volumen dinámico y gestión de pulsadores físicos sin saturar el bucle principal (`loop`). Además, el repositorio incluye de forma integrada todo el código fuente y las clases de soporte necesarias dentro de su estructura de carpetas local, por lo que no requiere la instalación manual de librerías externas adicionales ni descargas complementarias para compilar y funcionar de inmediato.


## 2. Instrucciones de Descarga y Clonación del Repositorio

Puedes obtener una copia de este proyecto en tu equipo local utilizando cualquiera de los dos métodos descritos a continuación:

### 2.1. Clonación mediante Git (Línea de comandos)
Si cuentas con Git instalado en tu computadora, abre tu terminal o consola de comandos y ejecuta la siguiente instrucción para clonar el repositorio completo:

```bash
git clone [https://github.com/J-Jair/ESP32_DFPlayer_Modular.git](https://github.com/J-Jair/ESP32_DFPlayer_Modular.git)

```
### 2.2. Descarga directa como archivo ZIP
Si prefieres no utilizar la terminal, puedes descargar el proyecto empaquetado siguiendo estos pasos:

1. Dirígete a la página principal del repositorio en el navegador web: [https://github.com/J-Jair/ESP32_DFPlayer_Modular](https://github.com/J-Jair/ESP32_DFPlayer_Modular).
2. Haz clic en el botón verde **"Code"** ubicado en la esquina superior derecha de la lista de archivos.
3. En el menú desplegable que aparece, selecciona la opción **"Download ZIP"**.
4. Una vez descargado el archivo comprimido en tu computadora, haz clic derecho sobre él, extráelo utilizando tu programa de descompresión preferido (como WinRAR, 7-Zip o la herramienta nativa de Windows) y tendrás la carpeta lista para usar en tu entorno de Arduino.

## 3. Requisitos de Hardware y Software

Para garantizar el correcto funcionamiento, compilación y despliegue del proyecto, asegúrate de contar con los siguientes componentes y herramientas:

### 3.1. Requisitos de Hardware
* **Microcontrolador:** Tarjeta de desarrollo **ESP32** (NodeMCU-32S o similar).
* **Módulo de Audio:** Módulo MP3 **DFPlayer Mini**.
* **Altavoz:** Altavoz de 3W a 8 Ohmios (conectado a los pines SPK_1 y SPK_2 del módulo).
* **Entradas de Control:** 3 pulsadores (botones táctiles momentáneos) para Play/Pausa, Siguiente y Anterior.
* **Componentes Pasivos:** 1 resistencia de 1 k$\Omega$ (para la línea de comunicación serial TX del DFPlayer).
* **Alimentación y Conexión:** Cables de prototipado (*jumpers*) y fuente de alimentación estable de 5V (o 3.3V según la especificación de tu placa ESP32, asegurando una correcta entrega de corriente para el reproductor de audio).

### 3.2. Requisitos de Software
* **IDE de Desarrollo:** [Arduino IDE](https://www.arduino.cc/) (versión 1.8.x o superior / Arduino IDE 2.x recomendado).
* **Soporte de Placa:** Paquete de tarjetas **ESP32** instalado en el Administrador de Tarjetas de Arduino (desarrollado por *Espressif Systems*).
* **Dependencias:** **Ninguna instalación externa requerida**. Todas las librerías, clases de abstracción y archivos de cabecera necesarios (`DFPlayerModule` y `DFRobotDFPlayerMini`) se encuentran incluidos localmente dentro de la estructura de carpetas del repositorio.

## 4. Diagrama de Conexiones de Hardware

A continuación se detallan las conexiones físicas requeridas entre el ESP32, el módulo DFPlayer Mini, el amplificador/altavoz y los pulsadores de control.

```text
    +---------------+                    +-------------------+
    |     ESP32     |                    |   DFPlayer Mini   |
    |               |                    |                   |
    |  GPIO 17 (TX) |------------------->| RX                |
    |  GPIO 16 (RX) |<--[ R 1k ]---------| TX                |
    |               |                    |                   |
    |               |                    | VCC ---------+----|-> 5V / 3.3V
    |  GND          |--------------------| GND ------+-------|-> GND Común
    |               |                    |                   |
    |               |                    | SPK_1 ----+       |
    |               |                    |           |-------|-> Altavoz (3W, 4-8Ω)
    |               |                    | SPK_2 ----+       |
    +---------------+                    +-------------------+
    |  GPIO 5       |--------+---> Botón Play/Pausa (con Pull-Up interno)
    |  GPIO 23      |--------+---> Botón Siguiente (con Pull-Up interno)
    |  GPIO 22      |--------+---> Botón Anterior (con Pull-Up interno)
    +---------------+
  ```  
   ### Tabla de Pinout y Conexiones

| Componente | Pin del Componente | Pin del ESP32 | Descripción / Nota de Ingeniería |
| :--- | :--- | :--- | :--- |
| **DFPlayer Mini** | VCC | 3.3V o 5V | Alimentación del módulo (Se recomienda 5V a través de la línea VIN o regulador externo, con GND común). |
| **DFPlayer Mini** | GND | GND | Tierra común obligatoria para la estabilidad de la UART. |
| **DFPlayer Mini** | TX | GPIO 16 (RX2) | Transmisión del DFPlayer hacia el ESP32. Se recomienda resistencia en serie de $1\,\text{k}\Omega$ para protección. |
| **DFPlayer Mini** | RX | GPIO 17 (TX2) | Recepción del DFPlayer (comandos enviados desde el ESP32). |
| **DFPlayer Mini** | SPK_1 / SPK_2 | Altavoz (3W, 4–8 $\Omega$) | Salida directa analógica hacia el altavoz pasivo. |
| **Pulsador Principal** | Pin 1 | GPIO 5 | Control de Play/Pausa (configurado con `INPUT_PULLUP`). |
| **Pulsador Siguiente** | Pin 1 | GPIO 23 | Salto a la pista posterior (`INPUT_PULLUP`). |
| **Pulsador Anterior** | Pin 1 | GPIO 22 | Retroceso a la pista previa (`INPUT_PULLUP`). |

> **Nota Visual del Módulo:** Para referencia física de los pines de alimentación, bus UART y salidas de altavoz:
> ![DFPlayer Mini Pinout](https://dfimg.dfrobot.com/nobody/wiki/77048a25b85b6e29438244020e7237e1_0x0.png.webp)

## 5. Estructura de Archivos del Proyecto

El código fuente y los recursos se encuentran organizados de manera modular en las siguientes carpetas a nivel general:

```text
/ESP32_DFPlayer_Modular
│
├── Main/
│   └── DFPlayerMini/
         └──DFPlayerMini.ino     <- Archivo principal.
│
├── Modules/
│   ├── DFPlayerModule.h         <- Cabecera de la capa de abstracción.
│   └── DFPlayerModule.cpp       <- Métodos lógicos y botones.
│
└── Libraries/
    ├── DFRobotDFPlayerMini.h    <- Comandos de bajo nivel y tramas UART.
    ├── DFRobotDFPlayerMini.cpp  <- Protocolo de tramas y checksums.
    └── Datasheet/               <- Carpeta de documentación.
        └── FN-M16P+...pdf       <- Datos oficial del módulo.

```

## 6. Estructura y Descripción de Archivos del Proyecto

Para facilitar la organización, el acceso y la comprensión por parte del usuario, los códigos fuente y recursos se distribuyen de la siguiente manera dentro de la estructura real de carpetas:

### 6.1. Archivo Principal: `DFPlayerMini.ino`
* **Ubicación:** `/Main/DFPlayerMini/DFPlayerMini.ino`
* **Descripción general:** Es el sketch principal de ejecución. Se encarga de inicializar el microcontrolador ESP32, configurar los pines digitales de los botones físicos mediante resistencias de *pull-up* internas e invocar de forma cíclica los métodos de control de reproducción de audio.

### 6.2. Driver UART de Bajo Nivel (Implementación): `DFRobotDFPlayerMini.cpp`
* **Ubicación:** `/Libraries/DFRobotDFPlayerMini.cpp`
* **Descripción general:** Implementa el protocolo de comunicación serial de bajo nivel. Se encarga de empaquetar las tramas de comandos, calcular la suma de verificación (*checksum*), gestionar los tiempos de espera (*timeouts*) y analizar las respuestas o eventos devueltos por el módulo MP3.

### 6.3. Driver UART de Bajo Nivel (Cabecera): `DFRobotDFPlayerMini.h`
* **Ubicación:** `/Libraries/DFRobotDFPlayerMini.h`
* **Descripción general:** Archivo de cabecera encargado de definir las macros de configuración, códigos de comandos hexadecimales, estados, tipos de errores, perfiles de ecualización y la estructura completa de la clase base de la librería del fabricante adaptada para el manejo de tramas seriales.

### 6.4. Capa de Abstracción (Implementación): `DFPlayerModule.cpp`
* **Ubicación:** `/Modules/DFPlayerModule.cpp`
* **Descripción general:** Contiene la lógica detallada de los métodos del reproductor. Gestiona la comunicación UART por hardware (Serial 2 del ESP32), valida la disponibilidad del módulo DFPlayer Mini y traduce las acciones de los botones en comandos de audio concretos.

### 6.5. Capa de Abstracción (Cabecera): `DFPlayerModule.h`
* **Ubicación:** `/Modules/DFPlayerModule.h`
* **Descripción general:** Archivo de cabecera que define la clase `DFPlayerModule`. Declara todos los atributos, prototipos de funciones para el manejo de volumen, estados de reproducción, control de pistas y la gestión lógica de los pulsadores físicos.

### 6.6. Documentación Técnica de Soporte (Datasheet)
* **Ubicación:** `/Libraries/Datasheet/`
* **Descripción general:** Contiene la hoja de datos oficial en formato PDF del fabricante (`FN-M16P+Embedded+MP3+Audio+Module...`), provista como referencia técnica de hardware sobre especificaciones eléctricas, voltajes y diagramas de pines del módulo.

## 7. Guía de Uso, Modificación y Buenas Prácticas

1. **Estructura de la Tarjeta SD:** Para que los comandos secuenciales (`next()`, `previous()`) y directos funcionen de manera óptima, formatee su tarjeta SD en formato **FAT32** y organice los archivos de audio en formato `.mp3` renombrándolos de forma numérica estricta (ej. `0001.mp3`, `0002.mp3`) dentro de la raíz o dentro de carpetas nombradas con dos dígitos (`/01/001.mp3`).
2. **Modificación de Pines UART:** Si requiere cambiar los pines seriales por defecto en el ESP32, simplemente modifique los parámetros al instanciar el objeto en el archivo principal:
   ```cpp
   // Cambiando a RX en GPIO 3 y TX en GPIO 1 por ejemplo:
   DFPlayerModule dfPlayer(3, 1);

## 🤝 Colaboración y Aprendizaje

💡 Este repositorio es un proyecto de ingeniería en constante evolución, enfocado en el desarrollo de sistemas embebidos eficientes y modulares. Si encuentras un error, tienes alguna sugerencia de mejora, una optimización para el código o deseas proponer una nueva funcionalidad, no dudes en abrir un *Issue* o enviar una propuesta de mejora. 

## ¡El aprendizaje colaborativo y la experimentación son la base de la ingeniería! 🤝🛠️