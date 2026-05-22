#include <WiFi.h>
#include <HTTPClient.h>
#include <time.h> 
#include <ArduinoJson.h> 
#include "DHT.h"

// ================= Configurações de Rede e API =================
const char* ssid = "NOME_DA_SUA_REDE_WIFI";
const char* password = "SENHA_DA_SUA_REDE_WIFI";

// Configurações do CallmeBot (WhatsApp)
String phoneNumber = "+55SEUNUMERO"; // Ex: +5571999999999
String apiKey = "SUA_API_KEY_AQUI";  // Chave recebida pelo WhatsApp

// Configurações da API Python (Backend)
String serverName = "http://SEU_IP_LOCAL:8000/leituras";

// Configuração do NTP (Time Server para gerar Timestamp ISO8601)
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -10800; // Fuso BR (UTC -3)
const int   daylightOffset_sec = 0;

// ================= Definição de Pinos =================
#define DHTPIN 4          // Pino digital do sensor DHT11/22
#define DHTTYPE DHT11     // Mude para DHT22 se for o modelo branco
#define PIR_PIN 5         // Pino digital do sensor de presença
#define LDR_PIN 34        // Pino analógico do fotorresistor
#define LED_VERDE 18      // LED de status normal
#define LED_VERMELHO 19   // LED de alerta

// Pinos da Ponte H (ex: L298N) para os 2 motores DC
#define IN1 25
#define IN2 26
#define IN3 27
#define IN4 14

DHT dht(DHTPIN, DHTTYPE);

// ================= Variáveis de Controle e Buffer =================
unsigned long ultimoTempo = 0;
bool alertaEnviado = false; // Evita spam no WhatsApp
bool roboLigado = true;     // Simula o controle remoto ligando/desligando

// Estrutura de Buffer (Armazena até 10 leituras se a rede cair)
#define TAMANHO_BUFFER 10
String bufferLeituras[TAMANHO_BUFFER];
int contagemBuffer = 0;

// ================= Protótipo das Funções =================
void enviarDados(float temp, float umid, int luz, int presenca, float prob);
void enviarAlertaWhatsApp();
void moverFrente();
void moverTras();
void pararMotores();

void setup() {
  Serial.begin(115200);
  dht.begin();
  
  pinMode(PIR_PIN, INPUT);
  pinMode(LDR_PIN, INPUT);
  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_VERMELHO, OUTPUT);
  
  // Configuração dos motores
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // Conexão Wi-Fi
  Serial.print("Conectando ao Wi-Fi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi conectado!");

  // Inicializa o relógio interno via servidor NTP
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

void loop() {
  // 1. Verificação de comandos externos via Serial (Simulação do controle remoto)
  if (Serial.available() > 0) {
    String comando = Serial.readStringUntil('\n');
    comando.trim();
    if (comando == "LIGAR") roboLigado = true;
    else if (comando == "DESLIGAR") roboLigado = false;
    else if (comando == "Frente") moverFrente();
    else if (comando == "Tras") moverTras();
    else pararMotores();
  }

  // 2. Se o robô foi desligado remotamente
  if (!roboLigado) {
    digitalWrite(LED_VERDE, LOW);
    digitalWrite(LED_VERMELHO, HIGH);
    pararMotores();
    return; // Interrompe o loop aqui
  }

  // 3. Leitura dos sensores a cada 2 segundos (não-bloqueante)
  if (millis() - ultimoTempo >= 2000) {
    ultimoTempo = millis();

    float temp = dht.readTemperature();
    float umidade = dht.readHumidity();
    int luz = analogRead(LDR_PIN); // Retorna valor de 0 a 4095
    bool presenca = digitalRead(PIR_PIN);

    // Validação da leitura do DHT
    if (isnan(temp) || isnan(umidade)) {
      Serial.println("Erro ao ler o sensor DHT!");
      return;
    }

    // 4. Cálculo de Probabilidade de Vida
    int probabilidadeVida = 0;

    if (temp >= 15.0 && temp <= 30.0) probabilidadeVida += 25;
    if (umidade >= 40.0 && umidade <= 70.0) probabilidadeVida += 25;
    if (luz > 1500) probabilidadeVida += 20; 
    if (presenca) probabilidadeVida += 30;

    // 5. Impressão no Monitor Serial
    Serial.printf("Temp: %.1f C | Umi: %.1f %% | Luz: %d | Presenca: %s\n", temp, umidade, luz, presenca ? "Sim" : "Nao");
    Serial.printf("Probabilidade de vida: %d %%\n", probabilidadeVida);

    // 6. Lógica de Decisão e Alerta
    if (probabilidadeVida <= 75) {
      digitalWrite(LED_VERDE, HIGH);
      digitalWrite(LED_VERMELHO, LOW);
      Serial.println("Exploracao normal. Nenhum indicio relevante detectado.");
      alertaEnviado = false; 
    } else {
      digitalWrite(LED_VERDE, LOW);
      digitalWrite(LED_VERMELHO, HIGH);
      Serial.println("ALERTA! Alta probabilidade de vida detectada!");
      
      if (!alertaEnviado) {
        enviarAlertaWhatsApp();
        alertaEnviado = true; 
      }
    }

    // 7. Salva a leitura no buffer e tenta enviar via HTTP POST para o Backend Python
    enviarDados(temp, umidade, luz, presenca, probabilidadeVida);

    Serial.println("--------------------------------------------------");
  }
}

// ================= Função de Envio de Dados para a API Python =================
void enviarDados(float temp, float umid, int luz, int presenca, float prob) {
  // 1. Gera o Timestamp ISO8601
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Falha ao obter tempo via NTP");
    return;
  }
  char timeStringBuff[50];
  strftime(timeStringBuff, sizeof(timeStringBuff), "%Y-%m-%dT%H:%M:%SZ", &timeinfo);
  
  // 2. Monta o JSON manualmente (Otimizado)
  String payload = "{";
  payload += "\"timestamp\": \"" + String(timeStringBuff) + "\",";
  payload += "\"temperatura_c\": " + String(temp) + ",";
  payload += "\"umidade_pct\": " + String(umid) + ",";
  payload += "\"luminosidade\": " + String(luz) + ",";
  payload += "\"presenca\": " + String(presenca) + ",";
  payload += "\"probabilidade_vida\": " + String(prob);
  payload += "}";

  // Adiciona a leitura atual ao Buffer se houver espaço
  if(contagemBuffer < TAMANHO_BUFFER) {
    bufferLeituras[contagemBuffer] = payload;
    contagemBuffer++;
  }

  // 3. Tenta enviar todo o conteúdo do Buffer para a API
  if(WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverName);
    http.addHeader("Content-Type", "application/json");

    int pacotesEnviados = 0;
    for(int i = 0; i < contagemBuffer; i++) {
      int httpResponseCode = http.POST(bufferLeituras[i]);
      if (httpResponseCode == 200) {
        pacotesEnviados++;
      } else {
        Serial.printf("Erro no POST da API. Codigo: %d\n", httpResponseCode);
        break; // Interrompe se o servidor falhar
      }
    }
    
    // Remove os pacotes enviados do buffer
    if(pacotesEnviados > 0) {
      for(int i = pacotesEnviados; i < contagemBuffer; i++) {
        bufferLeituras[i - pacotesEnviados] = bufferLeituras[i];
      }
      contagemBuffer -= pacotesEnviados;
      Serial.println("Dados enviados ao Banco SQLite com sucesso.");
    }
    http.end();
  } else {
    Serial.println("Erro de WiFi. Dados retidos no Buffer interno.");
  }
}

// ================= Função do CallmeBot =================
void enviarAlertaWhatsApp() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    // Montagem da URL
    String url = "https://api.callmebot.com/whatsapp.php?phone=" + phoneNumber + "&text=Alerta!+Alta+probabilidade+de+vida+detectada+no+planeta.&apikey=" + apiKey;
    
    http.begin(url);
    int httpResponseCode = http.GET();
    
    if (httpResponseCode == 200) {
      Serial.println("Mensagem de alerta enviada pelo CallmeBot!");
    } else {
      Serial.printf("Erro ao enviar mensagem. Codigo HTTP: %d\n", httpResponseCode);
    }
    http.end();
  } else {
    Serial.println("Erro: Wi-Fi desconectado, impossivel enviar alerta do WhatsApp.");
  }
}

// ================= Funções de Movimentação (Ponte H L298N) =================
void moverFrente() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
}

void moverTras() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
}

void pararMotores() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
}