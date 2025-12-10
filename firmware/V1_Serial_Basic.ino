// V1: Controle Básico via Serial

// Mapeamento dos pinos (D2, D5, D3, D6, D8 no ESP8266/NodeMCU)
#define X_STEP D2 // Pino de Passo (Pulso) do Eixo X
#define X_DIR D5  // Pino de Direção do Eixo X
#define Y_STEP D3 // Pino de Passo (Pulso) do Eixo Y
#define Y_DIR D6  // Pino de Direção do Eixo Y
#define EN D8     // Pino de Enable (Habilita Drivers)

#define PASSO_MM 0.01865 // Fator de conversão: milímetros por passo (baseado no setup do motor/microstep)
#define MICRO_DELAY 50   // Tempo de espera em microssegundos entre passos (controla a velocidade máxima)

long acumuladoX = 0; // Contagem absoluta de passos dados em X
long acumuladoY = 0; // Contagem absoluta de passos dados em Y
long homeX = 0;      // Posição de HOME salva em passos X
long homeY = 0;      // Posição de HOME salva em passos Y

void setup() {
  Serial.begin(115200);
  // Configura todos os pinos como SAÍDA
  pinMode(X_STEP, OUTPUT);
  pinMode(X_DIR, OUTPUT);
  pinMode(Y_STEP, OUTPUT);
  pinMode(Y_DIR, OUTPUT);
  pinMode(EN, OUTPUT);

  // Habilita os drivers (LOW para a maioria dos shields CNC)
  digitalWrite(EN, LOW);
  // Define uma direção inicial
  digitalWrite(X_DIR, HIGH);
  digitalWrite(Y_DIR, HIGH);
}

void moverPassos(int pinoStep, int dirPin, long passos, bool paraFrente, long *acum) {
  // Define a direção: HIGH ou LOW
  digitalWrite(dirPin, paraFrente ? HIGH : LOW);
  
  for (long i = 0; i < passos; i++) {
    // Sequência de pulso: HIGH -> LOW
    digitalWrite(pinoStep, HIGH);
    delayMicroseconds(MICRO_DELAY); // Tempo do pulso HIGH
    digitalWrite(pinoStep, LOW);
    delayMicroseconds(MICRO_DELAY); // Tempo entre pulsos (controla a velocidade)
    
    // Atualiza o contador acumulado de passos
    *acum += paraFrente ? 1 : -1;
  }
}

void moverParaXY(float x_mm, float y_mm) {
  // 1. Calcula a posição alvo em passos absolutos (usando o ponto HOME como referência)
  long alvoX = homeX + round(x_mm / PASSO_MM);
  long alvoY = homeY + round(y_mm / PASSO_MM);

  // 2. Calcula quantos passos são necessários
  long passosX = alvoX - acumuladoX;
  long passosY = alvoY - acumuladoY;
  
  // Como esta versão não tem coordenação de eixos (movimento diagonal),
  // ele move X completamente e depois move Y completamente.
  
  // Move X (usa valor absoluto, a função moverPassos lida com a direção)
  moverPassos(X_STEP, X_DIR, abs(passosX), passosX > 0, &acumuladoX);
  // Move Y
  moverPassos(Y_STEP, Y_DIR, abs(passosY), passosY > 0, &acumuladoY);
  
  Serial.print("Movido para X=");
  Serial.print(x_mm);
  Serial.print(" mm, Y=");
  Serial.println(y_mm);
}

void moverParaHome() {
  // Simplesmente move para a coordenada 0,0 mm em relação ao HOME salvo
  moverParaXY(0, 0);
}

void loop() {
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim();

    if (input == "s") { // Salvar home
      homeX = acumuladoX;
      homeY = acumuladoY;
      Serial.println("Home salvo (0,0)!");
    } else if (input == "h") { // Voltar home
      moverParaHome();
    } else if (input == "E") { // Mostrar posição (em mm)
      Serial.print("X: ");
      Serial.print((acumuladoX - homeX) * PASSO_MM);
      Serial.print(" mm, Y: ");
      Serial.println((acumuladoY - homeY) * PASSO_MM);
    } else if (input.indexOf(',') > 0) { // Coordenadas X,Y em milímetros
      int sep = input.indexOf(',');
      float x_mm = input.substring(0, sep).toFloat();
      float y_mm = input.substring(sep + 1).toFloat();
      moverParaXY(x_mm, y_mm);
    } else if (input == "Q") { // Desenha um quadrado de 60cm x 60cm
      float tamanho = 600.0; // 60 cm em mm
      // Pega a posição atual (em mm, relativa ao home)
      float startX = (acumuladoX - homeX) * PASSO_MM;
      float startY = (acumuladoY - homeY) * PASSO_MM;
      
      // Sequência de movimentos do quadrado
      moverParaXY(startX + tamanho, startY);
      moverParaXY(startX + tamanho, startY + tamanho);
      moverParaXY(startX, startY + tamanho);
      moverParaXY(startX, startY);
      Serial.println("Quadrado concluído!");
    }
  }
}
