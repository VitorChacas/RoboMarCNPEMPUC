#include <AccelStepper.h>
#include <MultiStepper.h> // Para agrupar motores (movimento Gantry)
#include <ESP8266WiFi.h>  // Para criar o Access Point e o Web Server
#include <Servo.h>        // Biblioteca do servo

// ================== CONFIG WiFi ==================
const char* ssid = "ROBO_AP"; // Nome da rede WiFi
const char* password = "12345678";
WiFiServer server(80); // Servidor na porta HTTP padrão

// ================== PINOS DO CNC SHIELD ==================
// Driver X e A (Vertical), Driver Y e Z (Horizontal)
#define X_STEP D2
#define X_DIR D5
#define Y_STEP D3
#define Y_DIR D6
#define Z_STEP D4 // Eixo extra (Z)
#define Z_DIR D7
#define A_STEP D12 // Eixo extra (A)
#define A_DIR D13
#define EN D8 // Enable compartilhado
// Servo
#define SERVO_PIN D9
Servo servoBlue; // objeto para o servo

// ================== OBJETOS ==================
// AccelStepper para cada motor (modo DRIVER: STEP/DIR)
AccelStepper stepperX(AccelStepper::DRIVER, X_STEP, X_DIR);
AccelStepper stepperA(AccelStepper::DRIVER, A_STEP, A_DIR);
AccelStepper stepperY(AccelStepper::DRIVER, Y_STEP, Y_DIR);
AccelStepper stepperZ(AccelStepper::DRIVER, Z_STEP, Z_DIR);

// MultiStepper para mover eixos Gantry coordenados
MultiStepper eixoVertical;   // Agrupa X e A
MultiStepper eixoHorizontal; // Agrupa Y e Z

// ================== VARIÁVEIS DE POSIÇÃO ==================
// Array para armazenar a posição alvo de cada motor em passos (necessário para MultiStepper)
long posVertical[2] = {0, 0};   // [0]=X, [1]=A
long posHorizontal[2] = {0, 0}; // [0]=Y, [1]=Z

// ================== CONFIG GERAL ==================
void setup() {
  Serial.begin(115200);
  
  // Configura Enable e desabilita inicialmente (HIGH = desabilitado)
  pinMode(EN, OUTPUT);
  digitalWrite(EN, HIGH); 

  // Configuração dos motores: Define velocidade e aceleração (em passos/segundo e passos/segundo²)
  // Velocidades relativamente lentas para garantir que não patine no Gantry
  stepperX.setMaxSpeed(300);
  stepperX.setAcceleration(100);
  // ... (Configuração similar para A, Y, Z)

  // Configura direção correta (invertendo se necessário)
  stepperX.setPinsInverted(false, false, false);
  stepperA.setPinsInverted(true, false, false); // Exemplo de inversão para Gantry espelhado
  // ... (Configuração similar para Y, Z)
  
  // Agrupa motores nos eixos (MultiStepper)
  eixoVertical.addStepper(stepperX);
  eixoVertical.addStepper(stepperA);
  eixoHorizontal.addStepper(stepperY);
  eixoHorizontal.addStepper(stepperZ);

  // Cria rede WiFi (Access Point)
  WiFi.softAP(ssid, password);
  Serial.print("Rede WiFi criada: ");
  Serial.println(ssid);
  Serial.print("IP: ");
  Serial.println(WiFi.softAPIP()); // O IP será 192.168.4.1
  server.begin();

  // Configura servo
  servoBlue.attach(SERVO_PIN);
  servoBlue.write(90); // Posição inicial neutra (meio)
}

// ================== FUNÇÕES DE MOVIMENTO ==================
void moverVertical(long passos) {
  // Atualiza as posições alvo dos dois motores (X e A)
  posVertical[0] += passos; // X
  posVertical[1] += passos; // A
  
  digitalWrite(EN, LOW); // Habilita drivers
  Serial.print("Movendo vertical para X:");
  Serial.print(posVertical[0]);
  Serial.print(" A:");
  Serial.println(posVertical[1]);
  
  // Move os motores para a posição absoluta de forma coordenada
  eixoVertical.moveTo(posVertical);
  while(eixoVertical.run()) {} // Bloqueia até que ambos cheguem
  digitalWrite(EN, HIGH); // Desabilita drivers
}

void moverHorizontal(long passos) {
  // Lógica similar para Y e Z
  posHorizontal[0] += passos; // Y
  posHorizontal[1] += passos; // Z
  
  digitalWrite(EN, LOW);
  // ... (Serial prints)
  
  eixoHorizontal.moveTo(posHorizontal);
  while(eixoHorizontal.run()) {}
  digitalWrite(EN, HIGH);
}

// ================== FUNÇÃO CRUZ 1 CM ==================
void fazerCruz() {
  long passosMetade = 114; // Passos necessários para mover 5 mm
  Serial.println("Iniciando cruz de 1 cm...");
  
  servoBlue.write(180); // Abaixa o servo
  delay(500); 

  // Sequência de movimentos (direita, centro, esquerda, centro, etc.)
  moverHorizontal(passosMetade); // direita
  delay(200); 
  moverHorizontal(-passosMetade); // volta ao centro
  // ... (outros movimentos)

  moverVertical(passosMetade); // cima
  delay(200);
  // ... (outros movimentos)

  servoBlue.write(0); // Levanta o servo (posição 0°)
  delay(500);
  Serial.println("Cruz de 1 cm concluída!");
}

// ================== LOOP PRINCIPAL ==================
void loop() {
  WiFiClient client = server.available(); // Verifica se há um novo cliente (browser)
  if (!client) return;

  Serial.println("Novo cliente conectado");
  String request = client.readStringUntil('\r'); // Lê a primeira linha da requisição HTTP
  client.flush();

  // Processa a requisição (comandos recebidos via URL, ex: /up, /cruz)
  if (request.indexOf("/up") != -1) {
    moverVertical(200);
  } else if (request.indexOf("/down") != -1) {
    moverVertical(-200);
  } // ... (outros comandos /right, /left, /cruz)

  // Página HTML estilo joystick (o ESP devolve uma página simples com botões)
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();
  // ... (código HTML dos botões e links)
  client.println("</html>");
}
