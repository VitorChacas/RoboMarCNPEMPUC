### **🤖 RoboMarCNPEMPUC — Plataforma Gantry Coordenada (ESP8266 + Python)**
**Desenvolvido por: _Vitor Pacheco_**

---

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
![Status](https://img.shields.io/badge/Status-V5%20Stable-brightgreen)
![Python](https://img.shields.io/badge/Python-3.10+-blue)
![C++](https://img.shields.io/badge/Firmware-Arduino%20C++-orange)
![ESP8266](https://img.shields.io/badge/Hardware-ESP8266-blue)
![UDP](https://img.shields.io/badge/Protocol-UDP%20High%20Speed-purple)

</div>

---

# 📌 **Descrição Geral**

O projeto **RoboMarCNPEMPUC** implementa um sistema robótico estilo **Gantry CNC**, controlado por:

- **ESP8266 (firmware C++)**
- **Python GUI (controle e automação)**
- Comunicação **UDP de alto desempenho**

O sistema controla **4 motores de passo coordenados** e **2 servos**, permitindo movimentos suaves e sincronizados.

---

# 🧩 **Arquitetura do Sistema**

+-------------------+ UDP/Wi-Fi +--------------------+
| PC / Python GUI | <-------------------> | ESP8266 (Firmware) |
| Interface & G-Code| | Executor de Movimento |
+-------------------+ +--------------------+
|
v
+-------------------------+
| CNC Shield V3 |
| X / Y / Z / A Motors |
+-------------------------+

---

# 🛠️ **Hardware Utilizado**

| Componente | Função |
| :--- | :--- |
| **ESP8266** (NodeMCU/Wemos D1 mini) | Controlador principal |
| **CNC Shield V3** | Driver dos motores |
| **4x Motores de Passo** | Eixos X, Y, Z, A |
| **2x Servos** | Caneta/Ferramenta |
| **Fonte 12V** | Alimentação |

---

# 📡 **Mapeamento da Comunicação (V5)**

| Componente | Protocolo | Porta | Endereço |
| :--- | :--- | :--- | :--- |
| Firmware ESP8266 | UDP | 8888 | 192.168.4.1 |
| PC/GUI Python | UDP Client | variável | conecta ao AP |

---

# 🎮 **Configuração dos Eixos**

| Eixo Lógico | Motores Físicos | Pinos | Descrição |
| :--- | :--- | :--- | :--- |
| Vertical | X + A (espelhados) | D2 + D12 | Movimento Y da gantry |
| Horizontal | Y + Z (espelhados) | D3 + D4 | Movimento X da gantry |

---

# 🧪 **Instalação do Software**

### Firmware (Arduino IDE)
Bibliotecas:
- AccelStepper
- MultiStepper
- Servo

Caminho:
/firmware/V5_UDP_Coordinated

### GUI Python
pip install pandas openpyxl numpy
python software/python_gui/control_gui.py

🚀 Guia de Uso (Modo Produção – V5)
1️⃣ Conexão
Wi-Fi: ROBO_CNPEM
Senha: 12345678
2️⃣ Teste rápido
Abra a GUI → Conectar → enviar comando:
T1

3️⃣ Envio de movimentos coordenados
Formato:
Vx Dx Vy Dy Va Da Vz Dz

---

# 🧭 **Próxima Fase (Roadmap para Turmas Futuras)**

## 📘 **Estudo Obrigatório**
- **Interpolação – Bresenham (V3)**
  Arquivo: /firmware/V3_Blynk_Advanced/V3_Blynk_Advanced.ino
- **Controle Coordenado – AccelStepper (V5)**
  Arquivo: /firmware/V5_UDP_Coordinated/V5_UDP_Coordinated.ino
- **Automação via Python**
  Arquivo: /software/python_gui/control_gui.py

## 🛠️ **Objetivos sugeridos (V6 – Próxima Iteração)**

| Objetivo | Área | Descrição |
| :--- | :--- | :--- |
| Planejamento de Trajetória | Python | Interpretar G-Code → gerar vetores coordenados |
| Protocolo Confiável | Firmware | Implementar ACK/NACK via UDP |
| Simulação 3D | Python | Visualizar trajetórias antes da execução |
| Otimização Dinâmica | Firmware | Ajuste automático de aceleração/deceleração |

---

# 🗂️ **Histórico de Versões**

| Versão | Pasta | Destaque | Comunicação |
| :--- | :--- | :--- | :--- |
| V1 | /V1_Serial_Basic | Controle básico | Serial |
| V2 | /V2_WiFi_Web_Simple | MultiStepper | WebServer |
| V3 | /V3_Blynk_Advanced | Bresenham | Blynk IoT |
| V4 | /V4_WiFi_Web_FullCommand | 10 variáveis | WebServer |
| V5 | /V5_UDP_Coordinated | PRODUÇÃO | UDP |

---

## 👤 **Autor**
<div align="center">
Desenvolvido por **Vitor Pacheco**
Se quiser evoluir ou contribuir — PRs são bem-vindos!
</div>

## 📄 **Licença**
Licenciado sob a MIT License.
Veja o arquivo `LICENSE`.
