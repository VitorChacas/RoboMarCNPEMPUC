Markdown

# 📝 Formato do Comando Coordenado (Firmware V5 - UDP)

O firmware V5 opera primariamente através do protocolo UDP, esperando um comando de 8 variáveis que define o deslocamento e a velocidade máxima para os 4 motores de passo (X, Y, Z, A) em um único segmento de movimento coordenado.

## 1. Formato Coordenado (8 Variáveis)

Este formato é usado pelo script Python ao enviar as rotinas lidas da planilha (MT/MR). O parser do ESP8266 (função `parseTabDelimitedCommand`) lida com separadores de espaço ou tabulação.

**Sintaxe:**
Vx Dx Vy Dy Va Da Vz Dz


| Variável | Eixo | Tipo (C/Python) | Descrição | Unidade |
| :--- | :--- | :--- | :--- | :--- |
| **Vx** | X | Long/int | **V**elocidade Máxima do motor X (passos/s). | passos/s |
| **Dx** | X | Long/int | **D**eslocamento relativo em passos para o motor X. | passos |
| **Vy** | Y | Long/int | **V**elocidade Máxima do motor Y (passos/s). | passos/s |
| **Dy** | Y | Long/int | **D**eslocamento relativo em passos para o motor Y. | passos |
| **Va** | A | Long/int | **V**elocidade Máxima do motor A (passos/s). | passos/s |
| **Da** | A | Long/int | **D**eslocamento relativo em passos para o motor A. | passos |
| **Vz** | Z | Long/int | **V**elocidade Máxima do motor Z (passos/s). | passos/s |
| **Dz** | Z | Long/int | **D**eslocamento relativo em passos para o motor Z. | passos |

---

## 2. Comandos Simples (Manual)

Estes comandos são usados para movimentos pontuais, testes e controle dos servos. São enviados através do campo "Comando Manual" na GUI Python.

| Comando | Ação | Exemplo | Firmware V5 Função |
| :--- | :--- | :--- | :--- |
| **X[passos]** | Mover motor X em N passos. | `X1600` | `moveSingleStepper` |
| **Y[passos]** | Mover motor Y em N passos. | `Y-500` | `moveSingleStepper` |
| **Z[passos]** | Mover motor Z em N passos. | `Z100` | `moveSingleStepper` |
| **E[passos]** | Mover motor A (Eixo E) em N passos. | `E-1000` | `moveSingleStepper` |
| **S[angulo]** | Mover Servo 1 (c1) para N graus (0-180). | `S90` | `servo1.write()` |
| **A[angulo]** | Mover Servo 2 (c2) para N graus (0-180). | `A180` | `servo2.write()` |
| **C1** | Executar rotina de teste da Cruz. | `C1` | `fazerCruz()` |
| **T1** | Teste de conexão/status. | `T1` | Retorna status OK |
