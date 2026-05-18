# iot-esp32-robo-espacia
# IoT ESP32 - Robô Espacial 🚀

## 📋 Objetivo da Etapa
O objetivo desta etapa é o desenvolvimento e a simulação do firmware do controle remoto para um robô espacial utilizando o microcontrolador ESP32-S3. O sistema lê as entradas analógicas de um Joystick para determinar a direção de movimento, possui LEDs indicadores de status de comunicação e um botão de parada de emergência para desligar o robô remotamente.

---

## 🛠️ Lista de Componentes do Circuito
O circuito foi montado virtualmente utilizando os seguintes componentes:
* **1x** ESP32-S3 (Placa de desenvolvimento principal)
* **1x** Analog Joystick (Para controle de direção nos eixos X e Y)
* **1x** Micro Servo Motor (Simulando o atuador/movimento do robô)
* **1x** Push Button (Botão remoto de ligar/desligar)
* **1x** LED Verde (Status: Controle ativo e conectado)
* **1x** LED Vermelho (Status: Robô desligado ou desconectado)
* **2x** Resistores de 220 MΩ (Para limitação de corrente dos LEDs)
* **1x** Protoboard e Jumpers de conexão

---

## 🔗 Link do Projeto no Wokwi
Acesse a simulação interativa diretamente pelo navegador:
👉 [(https://wokwi.com/projects/463750998187914241)]

---

## 🚀 Como Rodar o Projeto no Wokwi

1. Abra o link do projeto listado acima.
2. Certifique-se de que o código na aba `diagram.json` e no arquivo principal está carregado corretamente.
3. Clique no botão **Play (Iniciar Simulação)** (ícone de triângulo verde).
4. **Testando o Joystick:** Clique e arraste a bolinha central do Joystick para as extremidades. Verifique o braço do Servo Motor se movendo e as mensagens como `Comando: Frente` ou `Comando: Esquerda` aparecendo no Monitor Serial.
5. **Testando o Botão de Emergência:** Clique no botão verde na protoboard. O sistema alternará o status:
   * O LED Verde apagará e o LED Vermelho acenderá.
   * O monitor exibirá a mensagem: `Comando enviado: DESLIGAR`.
   * O motor ignorará os comandos do joystick até que o botão seja pressionado novamente.
