# 🚗📷 ESP32-CAM para Coches Robot

Proyecto educativo de streaming de video en tiempo real usando ESP32-CAM para coches robot controlados por ESP32.

## 📋 Descripción del Proyecto

Sistema de visión remota para coches robot que permite visualizar en tiempo real lo que "ve" el coche a través de una aplicación móvil desarrollada en MIT App Inventor. El ESP32-CAM se conecta a un router WiFi dedicado y transmite video en formato MJPEG que puede visualizarse desde cualquier dispositivo en la misma red.

### Características

- ✅ Streaming de video en tiempo real (MJPEG)
- ✅ Resolución QVGA (320x240) optimizada para fluidez
- ✅ Comunicación serial entre ESP32-CAM y ESP32 del coche
- ✅ Aplicación móvil con App Inventor
- ✅ Router WiFi dedicado para el proyecto
- ✅ Envío automático de IP cada 5 segundos

## 🛠️ Materiales Necesarios

### Hardware
- 1x **ESP32-CAM AI-THINKER** con cámara OV2640
- 1x **Programador FTDI** (para cargar código al ESP32-CAM)
- 1x **ESP32 DevKit** (para el coche robot)
- 1x **Router WiFi** configurado como punto de acceso
- Cables Dupont (macho-macho y macho-hembra)
- Fuente de alimentación 5V para ESP32-CAM

### Software
- Arduino IDE 2.x
- Placa ESP32 instalada en Arduino IDE
- MIT App Inventor 2
- Navegador web (Chrome, Firefox, etc.)

## 📐 Diagrama de Conexiones

### Conexión ESP32-CAM ↔️ ESP32 Coche (UART)

```
ESP32-CAM                    ESP32 Coche
─────────────────────────────────────────
TX (GPIO 1 / U0TXD)    →    RX (GPIO 18)
RX (GPIO 3 / U0RXD)    ←    TX (GPIO 17)
GND                    ─    GND
```

### Programación del ESP32-CAM

```
FTDI                         ESP32-CAM
─────────────────────────────────────────
3.3V                    →    3.3V
GND                     →    GND
TX                      →    U0RXD
RX                      →    U0TXD
                             GPIO 0 → GND (al programar)
```

⚠️ **IMPORTANTE**: Conectar GPIO 0 a GND solo durante la carga del código. Desconectar después para funcionamiento normal.

## 🔧 Configuración del Router WiFi

El proyecto usa un **router dedicado** para evitar interferencias con otras redes.

### Configuración recomendada:
```
SSID: COCHES_JAUME
Password: 12345678
IP Router: 192.168.77.1
DHCP Pool: 192.168.77.10 - 192.168.77.220
Canal: 1 o 6 (evitar interferencias)
```

## 💻 Instalación del Código

### 1. Preparar Arduino IDE

1. Instalar el soporte para ESP32:
   - Ir a **Archivo → Preferencias**
   - En "Gestor de URLs Adicionales de Tarjetas", añadir:
     ```
     https://espressif.github.io/arduino-esp32/package_esp32_index.json
     ```
   - Ir a **Herramientas → Placa → Gestor de tarjetas**
   - Buscar "ESP32" e instalar la versión más reciente

2. Configurar la placa:
   - **Placa**: AI Thinker ESP32-CAM
   - **CPU Frequency**: 240MHz
   - **Flash Frequency**: 80MHz
   - **Flash Mode**: QIO
   - **Partition Scheme**: Huge APP (3MB No OTA)
   - **Puerto**: Seleccionar el puerto COM del FTDI

### 2. Cargar el código al ESP32-CAM

1. Abrir el proyecto `ESP32_CAM_COCHES_JAIME.ino`
2. Verificar que estén los 3 archivos:
   - `ESP32_CAM_COCHES_JAIME.ino` (principal)
   - `camera_pins.h` (pines de la cámara)
   - `app_httpd.cpp` (servidor web)
3. **Conectar GPIO 0 a GND** en el ESP32-CAM
4. Presionar el botón **RESET** del ESP32-CAM
5. Clic en **Subir** (→) en Arduino IDE
6. Esperar a que termine la carga
7. **Desconectar GPIO 0 de GND**
8. Presionar **RESET** nuevamente

### 3. Verificar funcionamiento

1. Abrir el **Monitor Serie** a 115200 baudios
2. Presionar RESET en el ESP32-CAM
3. Deberías ver:
   ```
   Conectando a WiFi COCHES_JAIME...
   Conectado IP: 192.168.77.XX
   CAM: 192.168.77.XX
   ```
4. Abrir un navegador y poner la IP: `http://192.168.77.XX/`
5. Deberías ver el video en tiempo real

## 📱 Aplicación App Inventor

### Configuración del WebViewer

El archivo `.aia` incluido ya tiene la configuración correcta:

**Propiedades del WebViewer:**
- ✅ **UsesLocation**: Activado
- ✅ **IgnoreSslErrors**: Activado

**Bloques principales:**
```
Cuando se recibe texto por Bluetooth que contiene "CAM:":
  - Extraer la IP
  - Establecer WebViewer.HomeUrl a "http://[IP]/"
  - Llamar WebViewer.GoHome
```

### Importar el proyecto

1. Ir a https://appinventor.mit.edu
2. **Proyectos → Importar proyecto (.aia) desde mi computadora**
3. Seleccionar el archivo `ESP32_CAM_Control.aia`
4. Compilar la APK o usar AI Companion para probar

## 🚀 Uso del Sistema

### Secuencia de arranque:

1. **Encender el router WiFi** y esperar que arranque completamente
2. **Alimentar el ESP32-CAM** (esperar ~10 segundos para conexión WiFi)
3. **Abrir la app móvil** y conectar por Bluetooth al coche
4. La app recibirá automáticamente la IP de la cámara
5. El video aparecerá en el WebViewer
6. ¡Listo para conducir con visión en primera persona!

### Verificación de problemas:

| Problema | Solución |
|----------|----------|
| No conecta a WiFi | Verificar SSID y contraseña en el código |
| Video no carga en app | Verificar que UsesLocation esté activado |
| IP no llega al coche | Revisar conexiones TX/RX (¡pueden estar cruzadas!) |
| Video muy lento | Reducir resolución a `FRAMESIZE_QQVGA` |
| Cámara no inicia | Verificar que GPIO 0 NO esté conectado a GND |

## 📊 Ajustes de Calidad

En el archivo `ESP32_CAM_COCHES_JAIME.ino`, línea 83:

```cpp
// Opciones de resolución (de menor a mayor calidad):
s->set_framesize(s, FRAMESIZE_QQVGA);  // 160x120 - Muy fluido
s->set_framesize(s, FRAMESIZE_QVGA);   // 320x240 - Recomendado
s->set_framesize(s, FRAMESIZE_VGA);    // 640x480 - Más calidad
s->set_framesize(s, FRAMESIZE_SVGA);   // 800x600 - Requiere buena red
```

## 🎓 Para el Docente

### Objetivos didácticos:
- Comprender sistemas embebidos y comunicación WiFi
- Aprender protocolos de comunicación (UART, HTTP, MJPEG)
- Desarrollar aplicaciones móviles con App Inventor
- Integrar hardware y software en un proyecto real
- Trabajo en equipo y resolución de problemas técnicos

### Sugerencias de ampliación:
- Añadir control de calidad de video desde la app
- Implementar captura de fotos
- Añadir sensor de temperatura/humedad
- Crear sistema de grabación de video
- Implementar detección de obstáculos con la cámara

## 🐛 Solución de Problemas Comunes

### Error de compilación: "esp_camera.h: No such file"
**Solución**: Instalar o actualizar el paquete ESP32 en el Gestor de Tarjetas

### Error: "Brownout detector was triggered"
**Solución**: Usar una fuente de alimentación de al menos 500mA a 5V

### La cámara muestra imagen negra
**Solución**: Verificar que el ribbon cable de la cámara esté bien conectado

### WiFi no conecta
**Solución**: 
- Verificar que el router esté encendido
- Comprobar SSID y contraseña
- Asegurarse de que el router use 2.4GHz (no 5GHz)

## 📄 Licencia

Este proyecto es de código abierto para uso educativo. Libre de usar, modificar y distribuir con fines educativos.

---

## 👨‍🏫 Autor

**Jaume Ferràndiz**  
j.ferrandiz@edu.gva.es  
Departamento de Electrónica  
IES Marcos Zaragoza  
Villajoyosa (Alicante)

Ciclo Formativo de Grado Superior - Mantenimiento Electrónico  
Comunidad Valenciana, España

---

### 📞 Contacto y Soporte

Si eres docente y quieres usar este proyecto en tu aula, ¡adelante! 
Para dudas o mejoras, abre un **Issue** en este repositorio.

---

**⭐ Si te ha sido útil este proyecto, dale una estrella en GitHub ⭐**
