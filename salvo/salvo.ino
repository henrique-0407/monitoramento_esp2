#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>


#define ECG_PIN 32  // Pino do ECG (ou outro pino ADC disponível)
#define bt 2        // Pino do botão
#define bt2 4
#define rele 23


const char* ssid = "DALGELA";
const char* password = "veta271284";
const char* serverName = "http://192.168.15.11:5050/mensagem/";

int leituras[200];  // Vetor para armazenar as leituras do ECG
unsigned long tempos[200];  // Vetor para armazenar os tempos das leituras

void setup() {
  Serial.begin(115200);  // Inicializa a comunicação serial
  pinMode(ECG_PIN, INPUT);  // Configura o pino do ECG como entrada
  pinMode(bt, INPUT);  // Configura o pino do botão como entrada
  pinMode(bt2,INPUT);
  pinMode(rele, OUTPUT);
  Serial.println("Monitoramento ECG iniciado...");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando ao WiFi...");
  }
  Serial.println("Conectado ao WiFi");
}

void loop() {
  int leitura = digitalRead(bt2);
  if(leitura == HIGH){
  digitalWrite(rele, LOW);
  }
  else{
    digitalWrite(rele, HIGH);
  }
  bool estado = digitalRead(bt);
  bool referencia = 0;

  if (estado == HIGH) {
    referencia = 1;
  }

  if (referencia == 1) {
    // Realiza a leitura do ECG e armazena as leituras no vetor
    for (int i = 0; i < 200; i++) {
      leituras[i] = analogRead(ECG_PIN);  // Leitura do ECG
      tempos[i] = millis();  // Registra o tempo atual (em milissegundos)
      Serial.print("Leitura: ");
      Serial.print(leituras[i]);
      Serial.print(" - Tempo: ");
      Serial.println(tempos[i]);

      delay(4);  // Atraso entre as leituras (ajuste conforme necessário)
    }

    // Envia os dados para o servidor via POST
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      http.begin(serverName);
      http.addHeader("Content-Type", "application/json");

      // Constrói o JSON para enviar
      DynamicJsonDocument doc(2048);
      JsonArray ecgArray = doc.createNestedArray("ECG");

      // Adiciona as leituras e tempos ao JSON
      for (int i = 0; i < 200; i++) {
        JsonObject leitura = ecgArray.createNestedObject();
        leitura["valor"] = leituras[i];
        leitura["tempo"] = tempos[i];
      }

      String jsonPayload;
      serializeJson(doc, jsonPayload);  // Converte o documento JSON para uma string

      int httpResponseCode = http.POST(jsonPayload);  // Envia os dados via POST

      if (httpResponseCode > 0) {
        String response = http.getString();
        Serial.println("Resposta do servidor: " + response);
      } else {
        Serial.print("Erro ao enviar mensagem: ");
        Serial.println(httpResponseCode);
      }

      http.end();  // Finaliza a requisição
    } else {
      Serial.println("Erro de conexão WiFi");
    }

    // Aguarda um tempo antes de começar outra leitura
    referencia = 0;
    delay(5000);  // Aguarda 5 segundos
  }
}
