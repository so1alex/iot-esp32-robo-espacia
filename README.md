# Robô Explorador Espacial (IoT) 🚀

Projeto de um robô explorador físico que coleta dados ambientais e calcula a probabilidade de vida utilizando microcontroladores ESP32 e uma API Python.

## Estrutura do Repositório
- `/controle_remoto`: Código em `.ino` do Wokwi (Etapa 1).
- `/robo_fisico`: Firmware em C++ para o ESP32 do laboratório (Etapa 2).
- `/backend`: API FastAPI (`api.py`) e arquivo SQLite (`schema.sql`) (Etapa 3).

## Como montar o Robô (Hardware)
1. Conecte os motores DC aos pinos IN1 (25), IN2 (26), IN3 (27) e IN4 (14) da Ponte H.
2. O sensor DHT deve ser conectado ao pino 4, PIR no pino 5, e o LDR (divisor de tensão) no pino 34.
3. Os LEDs Verde e Vermelho devem ser conectados aos pinos 18 e 19, respectivamente.

## Como rodar o Backend Python
1. Instale as dependências: `pip install fastapi uvicorn pydantic`
2. Navegue até a pasta do backend.
3. Execute o servidor: `uvicorn api:app --host 0.0.0.0 --port 8000`

## Como consultar dados salvos
Para visualizar as últimas 100 leituras, acesse a rota GET pelo navegador:
`http://localhost:8000/leituras`