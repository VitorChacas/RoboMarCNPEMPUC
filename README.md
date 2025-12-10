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
### Instala as libs para leitura de Excel (pandas/openpyxl) e manipulação de dados (numpy)
pip install pandas openpyxl numpy

---

Guia de Uso (Versão de Produção - V5)
Passo 1: Configuração e Conexão
Grave o Firmware: Compile e grave o arquivo /firmware/V5_UDP_Coordinated/V5_UDP_Coordinated.ino no ESP8266.

Conecte-se à Rede: Conecte o seu computador à rede Wi-Fi criada pelo robô: ROBO_CNPEM (senha: 12345678).

Inicie o Controle: Execute a GUI em Python: python software/python_gui/control_gui.py

Teste a Conexão: Clique em "Conectar" e envie o comando simples T1.

Passo 2: Comandos e Rotinas
Comando Coordenado: O Host envia um string formatada: Vx Dx Vy Dy Va Da Vz Dz. Consulte docs/command_format.md para detalhes.

Rotinas de Planilha: Use os botões "Carregar Planilha" e "Enviar MT" / "Enviar MR" para automatizar o envio das sequências de movimento (lidas das linhas 18 e 19 do Excel).

📚 Estudo para a Próxima Turma: Focos de Código
Seu repositório oferece excelente material didático. Concentre-se nos seguintes pontos:

1. Interpolação de Trajetória
Referência: /firmware/V3_Blynk_Advanced/V3_Blynk_Advanced.ino

Estudo: Analisar a função moverParaXY() que implementa o Algoritmo de Bresenham para converter uma linha (trajeto em mm) em pulsos discretos e sincronizados para os motores.

2. Controle Coordenado de 4 Motores
Referência: /firmware/V5_UDP_Coordinated/V5_UDP_Coordinated.ino

Estudo: Focar em moverMotoresSimultaneos() e o loop while (stepperX.isRunning() | ...) para entender como a AccelStepper gerencia o tempo para que todos os 4 eixos cheguem ao destino simultaneamente, garantindo a fidelidade da trajetória.

3. I/O e Automação
Referência: /software/python_gui/control_gui.py

Estudo: Analisar a função _enviar_curva() para entender como o Pandas é usado na engenharia para ler dados brutos de calibração (MT/MR) em uma planilha e transformá-los em comandos de robô, fechando o ciclo de automação.
