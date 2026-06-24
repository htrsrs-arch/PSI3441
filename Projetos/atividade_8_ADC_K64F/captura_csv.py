import serial
import re
import csv

SERIAL_PORT = 'COM8'
BAUD_RATE = 115200

TOTAL_AMOSTRAS_ALVO = 400

tipo_ensaio = input("Digite 'bruto' ou 'filtrado' para identificar este ensaio: ").strip().lower()
NOME_ARQUIVO = f"dados_{tipo_ensaio}.csv"

data_regex = re.compile(
    r"T:\s*(\d+),\s*CH0R:\s*([\d.-]+),\s*CH0F:\s*([\d.-]+)"
)

try:
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=0.1)
    print(f"Conectado à porta {SERIAL_PORT}. Gravando EXATAMENTE {TOTAL_AMOSTRAS_ALVO} amostras em '{NOME_ARQUIVO}'...")
except Exception as e:
    print(f"Erro ao abrir a porta serial: {e}")
    exit()

with open(NOME_ARQUIVO, mode='w', newline='', encoding='utf-8') as arquivo_csv:
    escritor = csv.writer(arquivo_csv)
    escritor.writerow(['Timestamp_ms', 'CH0_Raw_V', 'CH0_Filt_V'])

    contador = 0
    while contador < TOTAL_AMOSTRAS_ALVO:
        if ser.in_waiting > 0:
            line = ser.readline().decode('utf-8', errors='ignore').strip()
            match = data_regex.search(line)

            if match:
                t    = int(match.group(1))
                ch0r = float(match.group(2))
                ch0f = float(match.group(3))

                escritor.writerow([t, ch0r, ch0f])
                contador += 1

                if contador % 40 == 0:
                    print(f"Progresso: {int((contador/TOTAL_AMOSTRAS_ALVO)*100)}% ({contador}/{TOTAL_AMOSTRAS_ALVO})")

print(f"\n[SUCESSO] Ensaio '{tipo_ensaio}' gravado com sucesso! Arquivo '{NOME_ARQUIVO}' fechado.")
ser.close()
