import serial
import re
import matplotlib.pyplot as plt
from collections import deque
from matplotlib.animation import FuncAnimation

# --- CONFIGURAÇÕES DA PORTA SERIAL ---
SERIAL_PORT = 'COM8' # Altere para a sua porta se necessário
BAUD_RATE = 115200

MAX_SAMPLES = 100
time_data    = deque(maxlen=MAX_SAMPLES)
ch0_raw_data = deque(maxlen=MAX_SAMPLES)

# Expressões regulares ajustadas milimetricamente para o novo firmware
data_regex    = re.compile(r"T:\s*(\d+),\s*CH0R:\s*([\d.-]+)")
metrics_regex = re.compile(r"Taxa:\s*(\d+)\s*Hz\s*\|\s*Perdidas:\s*(\d+)")

try:
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
    print(f"Conectado com sucesso à porta {SERIAL_PORT}")
except Exception as e:
    print(f"Erro ao abrir a porta serial: {e}")
    exit()

# Configuração da Janela Gráfica
fig, ax = plt.subplots(figsize=(10, 5))
fig.suptitle("Monitoramento em Tempo Real - Sinal ADC Bruto", fontsize=14, fontweight='bold')

line_raw, = ax.plot([], [], 'b-', label='CH0 Bruto (V)', linewidth=1.5)

ax.set_ylabel("Tensão (V)")
ax.set_xlabel("Tempo (ms)")
ax.legend(loc="upper right")
ax.grid(True)

# Caixa amarela de texto para exibição das métricas dinâmicas do RTOS
metrics_text = ax.text(
    0.01, 0.97, "Aguardando métricas do firmware...",
    transform=ax.transAxes, verticalalignment='top', fontsize=10,
    fontfamily='monospace',
    bbox=dict(boxstyle='round,pad=0.4', facecolor='lightyellow', alpha=0.85, edgecolor='gray')
)

def init():
    ax.set_ylim(-0.1, 3.5) # Escala padrão para tensões de microcontroladores (0 a 3.3V)
    return line_raw, metrics_text

def update(frame):
    while ser.in_waiting > 0:
        try:
            line = ser.readline().decode('utf-8', errors='ignore').strip()

            # 1. Verifica se a linha recebida contém metadados de performance
            if "[METRICAS]" in line:
                m = metrics_regex.search(line)
                if m:
                    taxa     = int(m.group(1))
                    perdidas = int(m.group(2))
                    metrics_text.set_text(
                        f"Taxa de amostragem : {taxa} Hz\n"
                        f"Msgs perdidas (1s) : {perdidas}"
                    )
                    print(f"[RTOS Metrics] {line}")
                continue

            # 2. Verifica se a linha recebida contém dados de telemetria do ADC
            match = data_regex.search(line)
            if match:
                t    = int(match.group(1))
                ch0r = float(match.group(2))

                time_data.append(t)
                ch0_raw_data.append(ch0r)

        except Exception:
            pass

    # Atualiza as curvas na interface visual se houver novos dados
    if time_data:
        line_raw.set_data(list(time_data), list(ch0_raw_data))
        ax.set_xlim(time_data[0], time_data[-1])

    return line_raw, metrics_text

# Animação rodando a 50 FPS (interval=20 ms)
ani = FuncAnimation(fig, update, init_func=init, interval=20, blit=False, cache_frame_data=False)

plt.tight_layout()
plt.show()
ser.close()