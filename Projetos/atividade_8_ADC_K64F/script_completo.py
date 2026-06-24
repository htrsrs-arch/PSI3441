import serial
import re
import matplotlib.pyplot as plt
from collections import deque
from matplotlib.animation import FuncAnimation

# --- CONFIGURAÇÕES DA PORTA SERIAL ---
SERIAL_PORT = 'COM8'
BAUD_RATE = 115200

MAX_SAMPLES = 100
time_data     = deque(maxlen=MAX_SAMPLES)
ch0_raw_data  = deque(maxlen=MAX_SAMPLES)
ch0_filt_data = deque(maxlen=MAX_SAMPLES)
ch0_err_data  = deque(maxlen=MAX_SAMPLES)

data_regex = re.compile(
    r"T:\s*(\d+),\s*CH0R:\s*([\d.-]+),\s*CH0F:\s*([\d.-]+)"
)
metrics_regex = re.compile(
    r"Taxa[^:]*:\s*(\d+)\s*Hz[^:]*:\s*(\d+)"
)

try:
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
    print(f"Conectado com sucesso à porta {SERIAL_PORT}")
except Exception as e:
    print(f"Erro ao abrir a porta serial: {e}")
    exit()

fig, ax = plt.subplots(figsize=(10, 5))
fig.suptitle("Análise em Tempo Real: ADC Bruto vs. Filtrado vs. Erro", fontsize=14, fontweight='bold')

line_raw,  = ax.plot([], [], 'r--', label='CH0 Bruto (V)',        alpha=0.6)
line_filt, = ax.plot([], [], 'r-',  label='CH0 Filtrado FIR (V)', linewidth=2)
line_err,  = ax.plot([], [], 'k:',  label='Erro Absoluto (V)',     alpha=0.7)

ax.set_ylabel("Tensão (V)")
ax.set_xlabel("Tempo (ms)")
ax.legend(loc="upper right", fontsize='small')
ax.grid(True)

metrics_text = ax.text(
    0.01, 0.97, "Aguardando métricas...",
    transform=ax.transAxes, verticalalignment='top', fontsize=10,
    fontfamily='monospace',
    bbox=dict(boxstyle='round,pad=0.4', facecolor='lightyellow', alpha=0.85, edgecolor='gray')
)

def init():
    ax.set_ylim(-0.1, 3.5)
    return line_raw, line_filt, line_err, metrics_text

def update(frame):
    while ser.in_waiting > 0:
        try:
            line = ser.readline().decode('utf-8', errors='ignore').strip()

            match = data_regex.search(line)
            if not match:
                m = metrics_regex.search(line)
                if m:
                    taxa     = int(m.group(1))
                    perdidas = int(m.group(2))
                    metrics_text.set_text(
                        f"Taxa de amostragem : {taxa} Hz\n"
                        #f"Msgs perdidas (1s) : {perdidas}"
                    )
                    print(f"[Firmware] {line}")
                continue
            if match:
                t   = int(match.group(1))
                ch0r = float(match.group(2))
                ch0f = float(match.group(3))

                time_data.append(t)
                ch0_raw_data.append(ch0r)
                ch0_filt_data.append(ch0f)
                ch0_err_data.append(abs(ch0r - ch0f))

        except Exception:
            pass

    if time_data:
        line_raw.set_data(list(time_data),  list(ch0_raw_data))
        line_filt.set_data(list(time_data), list(ch0_filt_data))
        line_err.set_data(list(time_data),  list(ch0_err_data))
        ax.set_xlim(time_data[0], time_data[-1])

    return line_raw, line_filt, line_err, metrics_text

ani = FuncAnimation(fig, update, init_func=init, interval=20, blit=False, cache_frame_data=False)

plt.tight_layout()
plt.show()
ser.close()
