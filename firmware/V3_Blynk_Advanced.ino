// V3: Blynk e Algoritmo de Linha (Bresenham)

// ---- Blynk / WiFi ----
// Credenciais Blynk (substitua pelas suas!)
#define BLYNK_TEMPLATE_ID "..."
#define BLYNK_TEMPLATE_NAME "..."
#define BLYNK_DEVICE_NAME "ROBOCNPEM"
#define BLYNK_AUTH_TOKEN "..." 
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <WidgetTerminal.h> // Para comandos via Terminal Blynk

char ssid[] = "GRUPO123";
char pass[] = "123456789";
WidgetTerminal terminal(V0); // Terminal virtual Blynk no pino V0

// ---- Pinos motores ----
#define X_STEP D2
#define X_DIR D5
#define Y_STEP D3
#define Y_DIR D6
#define EN D8
#define PASSO_MM 0.01865 // Fator de conversão
#define MICRO_DELAY 50   // Tempo de espera entre passos (Velocidade)

// ---- Variáveis posição ----
long acumuladoX = 0;
long acumuladoY = 0;
long homeX = 0;
long homeY = 0;

// ---- Setup ----
void setup() {
  Serial.begin(115200);
  // Configuração dos pinos (similar ao V1)
  pinMode(X_STEP, OUTPUT);
  // ...
  digitalWrite(EN, LOW); // Habilita drivers
  
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass); // Conecta ao WiFi e ao servidor Blynk
  terminal.println("ROBO pronto. Digite /help");
  terminal.flush();
}

// ---- BLYNK Terminal (Processa comandos de texto) ----
BLYNK_WRITE(V0) {
  String s = param.asStr();
  s.trim();
  
  if (s == "/help") {
    // Lista de comandos...
    terminal.println("Comandos:");
    // ...
    return;
  }
  
  if (s.equalsIgnoreCase("BUM")) {
    // Comando perigoso: Apaga o firmware da flash!
    terminal.println("Apagando firmware... ESP inutilizável até regravar!");
    terminal.flush();
    delay(100);
    ESP.eraseFlash(); 
    return;
  }
  
  // Comandos para desenhar formas (Arco, Circulo, Quadrado)
  if (s.startsWith("Arc")) { 
    float r = s.substring(3).toFloat();
    executarSemiArco(r, 40);
    return;
  }
  // ... (Outros comandos de forma)

  if (s.indexOf(',') > 0) { // Comando de Coordenada X,Y
    int sep = s.indexOf(',');
    float x_mm = s.substring(0, sep).toFloat();
    float y_mm = s.substring(sep + 1).toFloat();
    moverParaXY(x_mm, y_mm);
    return;
  }
  
  terminal.println("Comando invalido. Use /help");
  terminal.flush();
}

// ---- Botões (V1 e V2 são pinos virtuais Blynk para botões) ----
BLYNK_WRITE(V1) { 
  if (param.asInt() == 1) { // Quando o botão "Salvar Home" é pressionado
    homeX = acumuladoX;
    homeY = acumuladoY;
    terminal.println("Home salvo (0,0)");
    terminal.flush();
  }
}

// ---- Loop ----
void loop() {
  Blynk.run(); // Necessário para manter a conexão Blynk ativa
}

// ---- Funções motor (V3 usa uma lógica diferente de V1 e V2) ----

// moverPassos é a mesma função de V1 (movimento bloqueante manual)

/**
 * @brief Implementa o algoritmo de interpolação digital (DDA/Bresenham)
 * para mover os eixos X e Y simultaneamente (movimento diagonal linear).
 */
void moverParaXY(float x_mm, float y_mm) {
  // Calcula posições alvo e diferenças em passos
  long alvoX = homeX + lround(x_mm / PASSO_MM);
  long alvoY = homeY + lround(y_mm / PASSO_MM);
  long dx = alvoX - acumuladoX;
  long dy = alvoY - acumuladoY;

  // Direções
  bool dirX = dx >= 0, dirY = dy >= 0;
  dx = labs(dx); // Valor absoluto
  dy = labs(dy);

  digitalWrite(X_DIR, dirX ? HIGH : LOW);
  digitalWrite(Y_DIR, dirY ? HIGH : LOW);

  long steps = max(dx, dy); // O número total de passos será o maior deslocamento
  long errX = 0, errY = 0;   // Variáveis de erro para a interpolação

  // Loop de Bresenham: Itera pelo maior número de passos
  for (long i = 0; i < steps; i++) {
    bool stepX = false, stepY = false;
    errX += dx;
    errY += dy;

    if (errX >= steps) { // Se o erro X exceder o limite, dê um passo em X
      errX -= steps;
      stepX = true;
    }
    if (errY >= steps) { // Se o erro Y exceder o limite, dê um passo em Y
      errY -= steps;
      stepY = true;
    }

    if (stepX) digitalWrite(X_STEP, HIGH);
    if (stepY) digitalWrite(Y_STEP, HIGH);
    delayMicroseconds(MICRO_DELAY); // Pulso HIGH

    if (stepX) {
      digitalWrite(X_STEP, LOW);
      acumuladoX += dirX ? 1 : -1;
    }
    if (stepY) {
      digitalWrite(Y_STEP, LOW);
      acumuladoY += dirY ? 1 : -1;
    }
    delayMicroseconds(MICRO_DELAY); // Pulso LOW
  }
  
  // Feedback da posição no terminal Blynk
  // ...
}

// ---- Funções de Formas Geométricas ----
void executarSemiArco(float radius_mm, int seg) {
  // Implementa um arco usando cálculos trigonométricos
  // Divide o arco em 'seg' segmentos e chama moverParaXY para cada ponto.
  // ... (cálculos de seno e cosseno)
}
// ... (executarCirculo, executarQuadrado análogos)
