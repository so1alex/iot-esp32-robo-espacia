// Definição dos Pinos
const int pinoJoyX = 1;
const int pinoJoyY = 2;
const int pinoBotao = 6;
const int ledVerde = 4;
const int ledVermelho = 5;
const int pinoServo = 18;

bool roboLigado = true;
bool ultimoEstadoBotao = HIGH;

void setup() {
  Serial.begin(115200);
  pinMode(pinoBotao, INPUT_PULLUP);
  pinMode(ledVerde, OUTPUT);
  pinMode(ledVermelho, OUTPUT);
  pinMode(pinoServo, OUTPUT);
  
  Serial.println("Controle Remoto Pronto!");
}

// Função para mover o motor sem travar o código (Pulsos manuais)
void moverMotor(int microssegundos) {
  digitalWrite(pinoServo, HIGH);
  delayMicroseconds(microssegundos);
  digitalWrite(pinoServo, LOW);
  // O delay de 20ms é o tempo de 'descanso' do servo
}

void loop() {
  // 1. Verificação do Botão (Liga/Desliga)
  bool estadoBotao = digitalRead(pinoBotao);
  if (estadoBotao == LOW && ultimoEstadoBotao == HIGH) {
    roboLigado = !roboLigado;
    if (!roboLigado) Serial.println("Comando enviado: DESLIGAR");
    else Serial.println("Comando enviado: LIGAR");
    delay(200); // Debounce
  }
  ultimoEstadoBotao = estadoBotao;

  // 2. Lógica de Controle
  if (roboLigado) {
    digitalWrite(ledVerde, HIGH);
    digitalWrite(ledVermelho, LOW);

    int x = analogRead(pinoJoyX);
    int y = analogRead(pinoJoyY);

    // Movimentação Baseada no Joystick
    if (y < 1000) { 
      Serial.println("Comando: Frente");
      moverMotor(2000); // Pulso longo
    } 
    else if (y > 3000) {
      Serial.println("Comando: Tras");
      moverMotor(1000); // Pulso curto
    } 
    else if (x < 1000) {
      Serial.println("Comando: Esquerda");
      moverMotor(1200);
    } 
    else if (x > 3000) {
      Serial.println("Comando: Direita");
      moverMotor(1800);
    } 
    else {
      moverMotor(1500); // Centro/Parado
    }
  } 
  else {
    // Sistema Desligado
    digitalWrite(ledVerde, LOW);
    digitalWrite(ledVermelho, HIGH);
    moverMotor(1500); // Mantém no centro
  }

  delay(20); // Mantém a frequência de 50Hz do servo
}
