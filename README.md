# RoboMarCNPEMPUC
Firmware e software de controle de um robô marcador de piso (ESP8266 (WEMOS) + Python).

# 🤖 Projeto ROBÔ CNPEM: Controle de Plataforma Gantry Coordenada (ESP8266 + Python)

Este repositório contém o firmware e o software de controle para uma plataforma robótica Gantry (CNC Shield), desenvolvida para mover 4 motores de passo (X, Y, Z, A) de forma coordenada e suave, com controle de ferramenta/caneta via Servos. O projeto utiliza comunicação **UDP/Wi-Fi** para comandos de alta velocidade e precisão, sendo a base fundamental para estudos de controle de movimento e cinemática.

## 🎯 Visão Geral do Sistema e Arquitetura

A arquitetura do projeto é baseada na comunicação sem fio entre um Host PC (executando Python) e o microcontrolador ESP8266, que atua como um Ponto de Acesso (Access Point - AP).

### Estrutura de Comunicação (V5)

| Componente | Função | Protocolo | Endereço |
| :--- | :--- | :--- | :--- |
| **Firmware** (ESP8266) | Mestre da Rede / Executor de Movimento | Wi-Fi AP + UDP (Porta 8888) | 192.168.4.1 |
| **Software** (Python GUI) | Cliente / Processador de Rotinas (Planilha) | UDP | Variável (Conecta ao AP) |

[Diagrama visual da montagem do Gantry (CNC Shield no ESP8266, motores, servos)]

### 🛠️ Configuração dos Eixos (Gantry Espelhado)

O sistema opera com um arranjo Gantry que exige a coordenação de eixos espelhados:

| Eixo Lógico | Motores Físicos | Pinos STEP | Finalidade |
| :--- | :--- | :--- | :--- |
| **Vertical** | X e A (Eixos espelhados) | D2 e D12 | Movimento Vertical da Gantry |
| **Horizontal** | Y e Z (Eixos espelhados) | D3 e D4 | Movimento Horizontal da Gantry |

---

## 💻 Requisitos de Hardware e Software

### 1. Hardware

| Componente | Detalhes | Pinos (Função) |
| :--- | :--- | :--- |
| **Microcontrolador** | ESP8266 (NodeMCU ou Wemos D1 mini) | |
| **Driver Board** | CNC Shield V3 | STEP/DIR (Motores), D8 (EN) |
| **Atuadores** | 4 x Motores de Passo | X, Y, Z, A/E |
| **Ferramenta** | 2 x Servo Motores | D9 (Servo 1/C1), D0 (Servo 2/C2) |

### 2. Software (Firmware)

Para compilar o código na pasta `/firmware/V5_UDP_Coordinated`, instale as seguintes bibliotecas no seu Arduino IDE:

* **`AccelStepper`** (Autor: Mike McCauley)
* **`MultiStepper`** (Geralmente incluída no AccelStepper)
* **`Servo`** (Padrão)

### 3. Software Host (Python)

Execute o comando no terminal para instalar as dependências do script `/software/python_gui/control_gui.py`:

```bash
# Instala as libs para leitura de Excel (pandas/openpyxl) e manipulação de dados (numpy)
pip install pandas openpyxl numpy
