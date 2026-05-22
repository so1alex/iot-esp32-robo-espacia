CREATE TABLE IF NOT EXISTS leituras (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    timestamp TEXT NOT NULL,
    temperatura_c REAL NOT NULL,
    umidade_pct REAL NOT NULL,
    luminosidade INTEGER NOT NULL,
    presenca INTEGER NOT NULL,
    probabilidade_vida REAL NOT NULL
);