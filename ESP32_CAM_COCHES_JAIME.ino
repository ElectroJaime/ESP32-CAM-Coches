#include "esp_camera.h"
#include <WiFi.h>
#define CAMERA_MODEL_AI_THINKER // Has PSRAM
#include "camera_pins.h"

// ==================== CONFIGURACIÓN WIFI - ROUTER DEDICADO ====================
const char *ssid = "COCHES_JAUME";
const char *password = "12345678";
// Router: 192.168.77.1
// Pool DHCP: 192.168.77.10 a 192.168.77.220
// La ESP32-CAM recibirá una IP dinámica del router

// ==================== CONFIGURACIÓN SERIAL PARA ENVIAR IP ====================
// El ESP32-CAM enviará su IP al coche por Serial
// Conexiones físicas:
// ESP32-CAM TX (GPIO 1 / U0TXD) → ESP32 Coche RX (GPIO 18)
// ESP32-CAM RX (GPIO 3 / U0RXD) → ESP32 Coche TX (GPIO 17)
// GND → GND

#define ENVIAR_IP_CADA_MS 5000  // Enviar IP cada 5 segundos

unsigned long ultimoEnvioIP = 0;

void startCameraServer();
void setupLedFlash(int pin);

void setup() {
  // ==================== INICIAR SERIAL ====================
  // Serial se usa tanto para debug como para enviar IP al coche
  Serial.begin(115200);
  Serial.setDebugOutput(false); // Desactivar mensajes de debug para no saturar
  
  // ==================== CONFIGURACIÓN DE CÁMARA ====================
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_UXGA;
  config.pixel_format = PIXFORMAT_JPEG;  // for streaming
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  // Configuración optimizada si hay PSRAM
  if (config.pixel_format == PIXFORMAT_JPEG) {
    if (psramFound()) {
      config.jpeg_quality = 10;
      config.fb_count = 2;
      config.grab_mode = CAMERA_GRAB_LATEST;
    } else {
      // Si no hay PSRAM, limitar resolución
      config.frame_size = FRAMESIZE_SVGA;
      config.fb_location = CAMERA_FB_IN_DRAM;
    }
  } else {
    config.frame_size = FRAMESIZE_240X240;
    #if CONFIG_IDF_TARGET_ESP32S3
    config.fb_count = 2;
    #endif
  }

  // ==================== INICIALIZAR CÁMARA ====================
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("ERROR_CAMARA: 0x%x\n", err);
    return;
  }

  // ==================== CONFIGURAR SENSOR ====================
  sensor_t *s = esp_camera_sensor_get();
  
  // Ajustes para OV3660
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1);        // Voltear verticalmente
    s->set_brightness(s, 1);   // Subir brillo
    s->set_saturation(s, -2);  // Bajar saturación
  }
  
  // Resolución inicial: QVGA (320x240) para fluidez en WiFi
  if (config.pixel_format == PIXFORMAT_JPEG) {
    s->set_framesize(s, FRAMESIZE_QVGA);
  }

  // Ajustes para otros modelos de cámara
  #if defined(CAMERA_MODEL_M5STACK_WIDE) || defined(CAMERA_MODEL_M5STACK_ESP32CAM)
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
  #endif

  #if defined(CAMERA_MODEL_ESP32S3_EYE)
  s->set_vflip(s, 1);
  #endif

  // Setup LED Flash si está disponible
  #if defined(LED_GPIO_NUM)
  setupLedFlash(LED_GPIO_NUM);
  #endif

  // ==================== CONECTAR A WIFI ====================
  WiFi.begin(ssid, password);
  WiFi.setSleep(false); // Importante para streaming fluido
  
  // Esperar conexión WiFi
  Serial.println("Conectando a WiFi COCHES_JAIME...");
  int intentos = 0;
  while (WiFi.status() != WL_CONNECTED && intentos < 40) {
    delay(500);
    Serial.print(".");
    intentos++;
  }
  Serial.println();

  // ==================== INICIAR SERVIDOR DE STREAMING ====================
  if (WiFi.status() == WL_CONNECTED) {
    startCameraServer();
    
    // Obtener y enviar IP al coche
    IPAddress myIP = WiFi.localIP();
    
    Serial.print("Conectado IP: ");
    Serial.println(myIP);
    
    // IMPORTANTE: Enviar la IP con salto de línea para que el coche la detecte
    Serial.print("CAM: ");
    Serial.println(myIP);
    
    // Pequeño delay para asegurar que se envía
    delay(100);
    
  } else {
    Serial.println("ERROR_WIFI");
  }
}

void loop() {
  // ==================== REENVIAR IP PERIÓDICAMENTE ====================
  // Cada ENVIAR_IP_CADA_MS milisegundos, reenviar la IP al coche
  // Esto asegura que aunque el coche se reinicie, reciba la IP
  
  unsigned long ahora = millis();
  
  if (ahora - ultimoEnvioIP >= ENVIAR_IP_CADA_MS) {
    ultimoEnvioIP = ahora;
    
    if (WiFi.status() == WL_CONNECTED) {
      IPAddress myIP = WiFi.localIP();
      Serial.print("CAM: ");
      Serial.println(myIP);
    }
  }
  
  // Pequeño delay para no saturar el loop
  delay(100);
}
