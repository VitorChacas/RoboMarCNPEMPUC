#include <ESP8266WiFi.h>
#include <AccelStepper.h>
#include <MultiStepper.h> 
#include <Servo.h>
#include <WiFiUdp.h> // Biblioteca essencial para comunicação UDP

// ================== CONFIG WiFi ==================
const char* ssid = "ROBO_CNPEM"; // SSID do Access Point
const char* password = "12345678"; 

// ================== CONFIG UDP ==================
WiFiUDP udp;
unsigned int localUdpPort = 8888; // Porta de escuta
char incomingPacket[512]; // Buffer para armazenar o comando recebido

// ================== PINOS DO CNC SHIELD ==================
// Mapeamento dos pinos (X e A = Vertical Gantry; Y e Z = Horizontal Gantry)
#define X_STEP D2 
#define X_DIR D5  
#define Y_STEP D3 
#define Y_DIR D6  
#define Z_STEP D4 
#define Z_DIR D7  
#define A_STEP D12 // Motor A (Eixo E no CNC Shield)
#define A_DIR D13
#define EN D8      
#define SERVO1_PIN D9 
#define SERVO2_PIN D0 

Servo servo1;
Servo servo2;

// ================== OBJETOS ==================
// AccelStepper para cada um dos 4 motores.
AccelStepper stepperX(AccelStepper::DRIVER, X_STEP, X_DIR);
AccelStepper stepperA(AccelStepper::DRIVER, A_STEP, A_DIR);
AccelStepper stepperY(AccelStepper::DRIVER, Y_STEP, Y_DIR);
AccelStepper stepperZ(AccelStepper::DRIVER, Z_STEP, Z_DIR);

String statusMessage = "Sistema pronto via UDP."; 

// ================== DECLARAÇÃO DE FUNÇÕES ==================
void parseAndExecuteCommand(String command);
// Função principal para parsear o comando de 8 valores (Vx Dx Vy Dy Va Da Vz Dz)
bool parseTabDelimitedCommand(String input, long &Vx, long &Dx, long &Vy, long &Dy, long &Va, long &Da, long &Vz, long &Dz);
void fazerCruz();
void sendUDPResponse(String message);
// Função que coordena o movimento dos 4 motores, ajustando velocidades dinamicamente
void moverMotoresSimultaneos(long Dx, long Da, long Dy, long Dz, long Vx, long Va, long Vy, long Vz);
void moveSingleStepper(AccelStepper &stepper, long steps);

// ================== CONFIG GERAL ==================
void setup() {
    Serial.begin(1153600); 
    Serial.println("Iniciando Sistema V5...");
    
    // Configura Enable e desabilita drivers inicialmente
    pinMode(EN, OUTPUT);
    digitalWrite(EN, HIGH); 

    // Configuração inicial de velocidade e aceleração (serão sobrescritas pelo comando UDP)
    stepperX.setMaxSpeed(1600);
    stepperX.setAcceleration(1600);
    // ... (configuração similar para A, Y, Z)

    // Configuração de direção (importante para Gantry espelhado)
    stepperX.setPinsInverted(false, false, false);
    stepperA.setPinsInverted(true, false, false); 
    stepperY.setPinsInverted(false, false, false);
    stepperZ.setPinsInverted(true, false, false); 

    // Configuração dos servos (c1 e c2)
    servo1.attach(SERVO1_PIN);
    servo2.attach(SERVO2_PIN);
    servo1.write(0); 
    servo2.write(0); 

    // Configuração WiFi em modo Access Point (O robô é o mestre da rede)
    WiFi.softAP(ssid, password);
    Serial.println("Rede WiFi criada: " + String(ssid));
    Serial.println("IP: " + WiFi.softAPIP().toString()); // Deve ser 192.168.4.1

    // Inicia servidor UDP
    if(udp.begin(localUdpPort)) {
        Serial.println("UDP Server iniciado na porta " + String(localUdpPort));
        statusMessage = "UDP OK - Aguardando comandos...";
    } else {
        Serial.println("ERRO ao iniciar UDP!");
        statusMessage = "ERRO UDP!";
    }
}

// ================== ROTINA DA CRUZ (Teste de Calibração) ==================
void fazerCruz() {
    statusMessage = "Executando Rotina da Cruz...";
    sendUDPResponse(statusMessage);
    
    long passosMetade = 700; 
    servo1.write(180); // Abaixa o servo (caneta)
    delay(500); 
    
    // Movimentos horizontais (Y e Z JUNTOS)
    // Velocidades 0 indicam que a velocidade max. atual (1600) será usada
    moverMotoresSimultaneos(0, 0, passosMetade/2, passosMetade/2, 0, 0, 0, 0); 
    delay(1000); 
    moverMotoresSimultaneos(0, 0, -passosMetade, -passosMetade, 0, 0, 0, 0); 
    delay(1000); 
    moverMotoresSimultaneos(0, 0, passosMetade/2, passosMetade/2, 0, 0, 0, 0); 
    delay(1000); 

    // Movimentos verticais (X e A JUNTOS)
    moverMotoresSimultaneos(passosMetade/2, passosMetade/2, 0, 0, 0, 0, 0, 0); 
    delay(1000); 
    moverMotoresSimultaneos(-passosMetade, -passosMetade, 0, 0, 0, 0, 0, 0); 
    delay(1000); 
    moverMotoresSimultaneos(passosMetade/2, passosMetade/2, 0, 0, 0, 0, 0, 0); 
    delay(1000); 

    servo1.write(0); // Levanta o servo
    delay(500);
    
    statusMessage = "Rotina da Cruz Concluída.";
    sendUDPResponse(statusMessage);
}

// ================== MOVIMENTO SIMPLES (Para X100, Y-200, etc.) ==================
void moveSingleStepper(AccelStepper &stepper, long steps) {
    digitalWrite(EN, LOW); 
    stepper.move(steps); 
    while (stepper.distanceToGo() != 0) {
        stepper.run();
        yield(); 
    }
    digitalWrite(EN, HIGH); 
}

// ================== MOVIMENTO SIMULTÂNEO DOS 4 MOTORES (Núcleo do V5) ==================
/**
 * @brief Executa o movimento coordenado dos 4 motores.
 * Os parâmetros V (Velocidade) permitem ajustar a taxa de avanço para cada segmento.
 * A coordenação é garantida pelo loop 'while(stepperX.isRunning() | ...)'
 */
void moverMotoresSimultaneos(long Dx, long Da, long Dy, long Dz, long Vx, long Va, long Vy, long Vz) {
    digitalWrite(EN, LOW); // Habilita Drivers

    // 1. Configura velocidades dinamicamente
    if (Vx > 0) stepperX.setMaxSpeed(Vx);
    if (Va > 0) stepperA.setMaxSpeed(Va);
    if (Vy > 0) stepperY.setMaxSpeed(Vy);
    if (Vz > 0) stepperZ.setMaxSpeed(Vz);

    // 2. Define o deslocamento (posição relativa)
    stepperX.move(Dx);
    stepperA.move(Da);
    stepperY.move(Dy);
    stepperZ.move(Dz);

    // 3. Executa em paralelo (todos os motores coordenados para chegar juntos)
    while (stepperX.isRunning() || stepperA.isRunning() || stepperY.isRunning() || stepperZ.isRunning()) {
        stepperX.run();
        stepperA.run();
        stepperY.run();
        stepperZ.run();
        yield();
    }
    digitalWrite(EN, HIGH); // Desabilita Drivers
}

// ================== PARSER PARA FORMATO COM TABULAÇÃO/ESPAÇO ==================
/**
 * @brief Analisa a string de 8 valores (Vx Dx Vy Dy Va Da Vz Dz) do Python.
 * Lida com separadores (espaço, tabulação) e vírgulas decimais.
 */
bool parseTabDelimitedCommand(String input, long &Vx, long &Dx, long &Vy, long &Dy, long &Va, long &Da, long &Vz, long &Dz) {
    input.trim();
    // Substitui separadores regionais e múltiplos espaços
    input.replace(',', '.');
    input.replace('\t', ' ');
    while (input.indexOf("  ") >= 0) { 
        input.replace("  ", " ");
    }
    input.trim();

    float values[8]; 
    int count = 0;
    // ... (lógica de tokenização e conversão para float/long)

    // Verifica se 8 valores foram recebidos
    if (count != 8) {
        Serial.printf("Erro: esperados 8 valores, recebidos %d\n", count);
        return false;
    }
    
    // Atribuição dos valores parseados às variáveis long de destino
    Vx = (long)values[0]; 
    Dx = (long)values[1];
    Vy = (long)values[2];
    Dy = (long)values[3];
    Va = (long)values[4];
    Da = (long)values[5];
    Vz = (long)values[6];
    Dz = (long)values[7];
    
    return true;
}

// ================== PARSER COMANDO SIMPLES ==================
void parseAndExecuteCommand(String command) {
    // ... (Lógica de switch case para X[passos], S[angulo], C1, T1, etc.)
    // ...
    // Serial.println(statusMessage);
    sendUDPResponse(statusMessage); // Envia status de volta ao cliente UDP
}

// ================== ENVIO RESPOSTA UDP ==================
void sendUDPResponse(String message) {
    // Envia a mensagem de feedback para o IP/Porta do cliente que enviou o último pacote
    if (udp.beginPacket(udp.remoteIP(), udp.remotePort())) {
        udp.write(message.c_str());
        udp.endPacket(); 
    }
}

// ================== EXECUTAR MOVIMENTO COORDENADO ==================
void executarMovimentoCoordenado(long Vx, long Dx, long Vy, long Dy, long Va, long Da, long Vz, long Dz) {
    Serial.println("=== INICIANDO MOVIMENTO COORDENADO ===");
    // Chama o núcleo do movimento
    moverMotoresSimultaneos(Dx, Da, Dy, Dz, Vx, Va, Vy, Vz);
    
    // Constrói e envia a mensagem de conclusão
    statusMessage = "Movimento concluído! X(V=" + String(Vx) + " D=" + Dx + ") ...";
    sendUDPResponse(statusMessage);
}

// ================== LOOP PRINCIPAL ==================
void loop() {
    int packetSize = udp.parsePacket(); // Verifica se há um comando UDP
    if (packetSize) {
        // Recebe o pacote
        int len = udp.read(incomingPacket, sizeof(incomingPacket) - 1);
        if (len > 0) {
            incomingPacket[len] = '\0';
            String command = String(incomingPacket);

            // 1. Verifica se é o comando coordenado de 8 valores (se houver tab/espaço)
            if (command.indexOf('\t') != -1 || (command.indexOf(' ') != -1 && command.length() > 10)) {
                long Vx, Dx, Vy, Dy, Va, Da, Vz, Dz;
                if (parseTabDelimitedCommand(command, Vx, Dx, Vy, Dy, Va, Da, Vz, Dz)) {
                    executarMovimentoCoordenado(Vx, Dx, Vy, Dy, Va, Da, Vz, Dz);
                } else {
                    statusMessage = "ERRO: Formato inválido! Use: Vx Dx Vy Dy Va Da Vz Dz";
                    sendUDPResponse(statusMessage);
                }
            } else {
                // 2. Se não, é um comando simples
                parseAndExecuteCommand(command);
            }
        }
    }
    delay(10); 
}
