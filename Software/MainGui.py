import tkinter as tk
from tkinter import ttk, filedialog, messagebox
import socket
import pandas as pd     # Para ler planilhas Excel
import time
import numpy as np
import math

# ================= CONFIG UDP =================
UDP_IP = "192.168.4.1"  # IP padrão do ESP8266 em modo Access Point
UDP_PORT = 8888         # Porta UDP (deve ser a mesma do V5)
sock = None             # Objeto socket
is_connected = False    # Estado da conexão
_df = None              # DataFrame para armazenar a planilha carregada

# ================= UTIL =================
def _to_number(v):
    """
    @brief Função utilitária para converter valores de célula (incluindo vírgulas e NaN)
    em float, garantindo que o parser lide com formatos regionais.
    """
    if v is None: return None
    # ... (lógica de conversão e tratamento de erro)
    s = str(v).strip().replace(',', '.')
    try:
        return float(s)
    except:
        return None

# ================= LOG =================
def log(msg):
    """
    @brief Adiciona mensagens ao Text Widget de Log (com timestamp).
    """
    txt_log.config(state='normal')
    txt_log.insert(tk.END, f"[{time.strftime('%H:%M:%S')}] {msg}\n")
    txt_log.see(tk.END)
    txt_log.config(state='disabled')

# ================= UDP COMMUNICATION =================
def connect_udp():
    """
    @brief Inicializa o socket UDP, define o timeout e envia um comando 'T1' para
    testar se o ESP8266 está respondendo.
    """
    global sock, is_connected
    try:
        if sock: sock.close()
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.settimeout(3.0) 
        sock.bind(('', 0)) # Liga a uma porta cliente
        
        # Envia teste 'T1'
        test_msg = "T1" 
        sock.sendto(test_msg.encode(), (UDP_IP, UDP_PORT))
        
        try:
            response, addr = sock.recvfrom(1024)
            log(f"✅ Conectado! Resposta: {response.decode()}")
            is_connected = True
            btn_connect.config(text="Conectado", style="Connected.TButton")
        except socket.timeout:
            log("⚠️ Socket criado mas sem resposta do ESP.")
            is_connected = False
            btn_connect.config(text="Conectar", style="TButton")
    except Exception as e:
        log(f"❌ Erro de conexão: {e}")
        is_connected = False
        btn_connect.config(text="Conectar", style="TButton")

def send_udp_command(cmd):
    """
    @brief Envia o comando e aguarda a resposta de feedback do firmware V5.
    """
    # ... (lógica de envio e recepção com tratamento de timeout)
    if not sock:
        log("❌ Socket não criado!")
        return None
    try:
        sock.sendto(cmd.encode(), (UDP_IP, UDP_PORT))
        log(f"▶️ Enviado: {cmd}")
        # ... (lógica de recepção)
    except Exception as e:
        # ... (tratamento de erro de envio/conexão)
        return None

# ================= COMANDOS SIMPLES E COORDENADOS =================
def send_manual():
    """ Envia comandos simples do campo de texto (Ex: X1000, S180, C1). """
    cmd = ent_manual.get().strip()
    if cmd:
        send_udp_command(cmd)

def enviar_comando_tab():
    """
    @brief Processa a entrada de 8 parâmetros (Vx Dx Vy Dy Va Da Vz Dz) e os envia
    ao firmware V5.
    """
    params = ent_params.get().strip()
    if not params:
        messagebox.showwarning("Aviso", "Digite 8 parâmetros.")
        return

    # Limpeza da string para lidar com tabulação, espaço e vírgula
    params_processed = params.replace('\t', ' ').replace(',', ' ')
    while '  ' in params_processed: 
        params_processed = params_processed.replace('  ', ' ')
    params_processed = params_processed.strip()

    values = params_processed.split()
    if len(values) != 8:
        messagebox.showerror("Erro", f"Esperados 8 valores, encontrados {len(values)}")
        return
    
    # Envia os valores separados por espaço (ou tab, ambos funcionam no V5)
    comando = " ".join(values) 
    log(f"📤 Enviando comando coordenado: {comando}")
    send_udp_command(comando)

# Funções de controle de servo e teste...
def fazer_cruz(): send_udp_command("C1") 
def servo1_on(): send_udp_command("S180")
def servo1_off(): send_udp_command("S0")
# ... (servo2_on/off, test_connection)

# ================= LEITURA DIRETA DA PLANILHA =================
def carregar_planilha():
    """
    @brief Abre o File Dialog e carrega a planilha Excel para o DataFrame Pandas.
    """
    global _df
    path = filedialog.askopenfilename( filetypes=[("Excel", "*.xlsx"), ("Excel", "*.xls")])
    if not path: return
    try:
        # header=None: Leitura bruta, 0-indexed a partir da primeira célula
        _df = pd.read_excel(path, header=None)
        log(f"📁 Planilha carregada: {path}")
    except Exception as e:
        messagebox.showerror("Erro", f"Erro lendo planilha: {e}")

def _enviar_curva(linha_indice, nome_curva):
    """
    @brief Função auxiliar para extrair e enviar os 8 valores da linha (MT ou MR).
    A curva MT é na linha 18 (índice 17) e MR na linha 19 (índice 18).
    Os dados relevantes estão nas colunas C a J (índices 2 a 9).
    """
    if _df is None:
        messagebox.showerror("Erro", "Carregue a planilha primeiro.")
        return
    try:
        valores = []
        for col in range(2, 10): # Colunas C (2) até J (9)
            val = _df.iloc[linha_indice, col]
            num_val = _to_number(val)
            
            if num_val is not None and not math.isnan(num_val):
                valores.append(num_val)
            else:
                messagebox.showerror("Erro", f"Valor inválido encontrado em {nome_curva}.")
                return

        if len(valores) == 8:
            valores_int = [int(round(v)) for v in valores]
            comando = " ".join(str(v) for v in valores_int) 
            
            log(f"🎯 Enviando curva {nome_curva}: (Vx Dx Vy Dy Va Da Vz Dz)")
            send_udp_command(comando)
        else:
            messagebox.showerror("Erro", f"Esperados 8 valores em {nome_curva}, encontrados: {len(valores)}")
            
    except Exception as e:
        messagebox.showerror("Erro", f"Erro lendo {nome_curva}: {e}")

def enviar_mt(): _enviar_curva(17, "MT") # Linha 18
def enviar_mr(): _enviar_curva(18, "MR") # Linha 19

def mostrar_valores_planilha():
    """ Exibe os valores lidos das linhas MT e MR da planilha em uma caixa de diálogo. """
    # ... (lógica de extração e exibição)
    pass

# ================= MOVIMENTOS INDIVIDUAIS =================
# Funções de wrapper para botões de movimento individual (X, Y, Z, E)
def mover_x():
    try:
        passos = int(ent_x.get())
        send_udp_command(f"X{passos}")
    except ValueError:
        messagebox.showerror("Erro", "Digite um número válido.")
# ... (mover_y, mover_z, mover_e análogos)

# ================= GUI =================
# ... (Configuração da janela, styles, frames e widgets Tkinter)
root = tk.Tk()
# ... (configuração visual)
# ... (mapeamento de botões para funções)

root.protocol("WM_DELETE_WINDOW", on_exit)
root.after(1000, connect_udp)
root.after(1500, lambda: ent_manual.focus())
root.mainloop()
