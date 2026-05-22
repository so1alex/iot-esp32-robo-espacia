from fastapi import FastAPI
from pydantic import BaseModel
import sqlite3

app = FastAPI(title="Robô Explorador API")

# Modelo de dados esperado no POST (O FastAPI valida isso automaticamente)
class Leitura(BaseModel):
    timestamp: str
    temperatura_c: float
    umidade_pct: float
    luminosidade: int
    presenca: int
    probabilidade_vida: float

# Função para conectar ao banco
def get_db_connection():
    conn = sqlite3.connect('banco_robo.db')
    conn.row_factory = sqlite3.Row
    return conn

# Inicializa o banco de dados com base no script SQL
@app.on_event("startup")
def startup():
    conn = get_db_connection()
    with open('schema.sql', 'r') as f:
        conn.executescript(f.read())
    conn.commit()
    conn.close()

# Rota POST: Recebe os dados do ESP32
@app.post("/leituras")
def salvar_leitura(leitura: Leitura):
    conn = get_db_connection()
    cursor = conn.cursor()
    cursor.execute('''
        INSERT INTO leituras (timestamp, temperatura_c, umidade_pct, luminosidade, presenca, probabilidade_vida)
        VALUES (?, ?, ?, ?, ?, ?)
    ''', (leitura.timestamp, leitura.temperatura_c, leitura.umidade_pct, leitura.luminosidade, leitura.presenca, leitura.probabilidade_vida))
    conn.commit()
    conn.close()
    return {"status": "sucesso", "mensagem": "Dados armazenados com sucesso."}

# Rota GET: Retorna as últimas 100 leituras
@app.get("/leituras")
def listar_leituras():
    conn = get_db_connection()
    cursor = conn.cursor()
    cursor.execute('SELECT * FROM leituras ORDER BY id DESC LIMIT 100')
    linhas = cursor.fetchall()
    conn.close()
    return [dict(linha) for linha in linhas]