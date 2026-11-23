
#include <Arduino.h>
#include "esp_camera.h"
#include <ESP32QRCodeReader.h>
#include <ESP32Servo.h>

#define SERVO_PIN 15  // Pino de sinal do servo motor (GPIO 14)

// Opcional: defina um texto esperado, ou deixe vazio "" para aceitar qualquer QR code valido
const String QR_ESPERADO = ""; 
const unsigned long TEMPO_BLOQUEIO_MS = 3000; // Evita repetir o mesmo QR code em seguida

ESP32QRCodeReader reader(CAMERA_MODEL_AI_THINKER);
Servo meuServo;

volatile bool acionando = false;
unsigned long ultimaLeituraMs = 0;
String ultimoConteudo = "";

void moverServo() {
  Serial.println("Acionando servo -> 90 graus...");
  meuServo.write(90);
  vTaskDelay(2000 / portTICK_PERIOD_MS);

  Serial.println("Retornando servo -> 0 graus...");
  meuServo.write(0);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
}

void onQrCodeTask(void *pvParameters) {
  struct QRCodeData qrCodeData;

  while (true) {
    if (!acionando && reader.receiveQrCode(&qrCodeData, 100)) {
      if (qrCodeData.valid) {
        String conteudo = String((char *)qrCodeData.payload);
        conteudo.trim();

        unsigned long agora = millis();
        bool mesmoQrRecente = (conteudo == ultimoConteudo) && (agora - ultimaLeituraMs < TEMPO_BLOQUEIO_MS);

        if (!mesmoQrRecente) {
          bool corresponde = (QR_ESPERADO.length() == 0) || (conteudo == QR_ESPERADO);

          if (corresponde) {
            acionando = true;
            ultimoConteudo = conteudo;
            ultimaLeituraMs = agora;

            Serial.println("==================================");
            Serial.print("QR Code Reconhecido: ");
            Serial.println(conteudo);
            Serial.println("==================================");

            moverServo();

            acionando = false;
          } else {
            Serial.print("QR Code ignorado (conteudo diferente): ");
            Serial.println(conteudo);
          }
        }
      }
    }
    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}
