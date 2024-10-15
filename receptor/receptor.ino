// Definição do pino de recepção
const int RX_PIN = 3;
const unsigned long BAUD_RATE = 9600;
const unsigned int BIT_DURATION = 1000000 / BAUD_RATE; // em microssegundos

void setup() {
  pinMode(RX_PIN, INPUT);
  Serial.begin(9600);
}

void loop() {
  if (digitalRead(RX_PIN) == LOW) {
    char recebido = receberUART();
    if (recebido != 0) {
      Serial.print("Caractere recebido: ");
      Serial.println(recebido);
    }
  }
}

char receberUART() {
  unsigned long startTime = micros();

  // Aguarda meio período para amostrar no meio do start bit
  waitForHalfBitTime(&startTime);

  // Confirma se ainda é start bit
  if (digitalRead(RX_PIN) != LOW) {
    return 0; // Erro na sincronização
  }

  // Avança para o próximo período de bit
  waitForNextBit(&startTime);

  byte data = 0;
  // Leitura dos 8 bits de dados
  for (int i = 0; i < 8; i++) {
    int bit = digitalRead(RX_PIN);
    data |= (bit << i);
    waitForNextBit(&startTime);
  }

  // Leitura do bit de paridade
  int parityBit = digitalRead(RX_PIN);
  waitForNextBit(&startTime);

  // Leitura do stop bit
  int stopBit = digitalRead(RX_PIN);
  if (stopBit != HIGH) {
    Serial.println("Erro no stop bit!");
    return 0; // Erro no stop bit
  }

  // Verificação da paridade
  byte paridadeCalculada = calcularParidade(data);
  if (parityBit != paridadeCalculada) {
    Serial.println("Erro de paridade!");
    return 0;
  }

  return data;
}

void waitForHalfBitTime(unsigned long* previousTime) {
  while (micros() - *previousTime < BIT_DURATION / 2) {
    // Aguarda meio período de bit
  }
  *previousTime += BIT_DURATION / 2;
}

void waitForNextBit(unsigned long* previousTime) {
  while (micros() - *previousTime < BIT_DURATION) {
    // Aguarda até que o tempo do bit tenha decorrido
  }
  *previousTime += BIT_DURATION;
}

byte calcularParidade(byte data) {
  byte paridade = 0;
  for (int i = 0; i < 8; i++) {
    paridade ^= (data >> i) & 0x01;
  }
  return paridade; // Paridade par
}
