// Definição do pino de transmissão
const int TX_PIN = 2;
const unsigned long BAUD_RATE = 9600;
const unsigned int BIT_DURATION = 1000000 / BAUD_RATE; // em microssegundos

void setup() {
  pinMode(TX_PIN, OUTPUT);
  digitalWrite(TX_PIN, HIGH);
  Serial.begin(9600);
}

void loop() {
  char caractere = 'B'; // Caractere a ser enviado
  enviarUART(caractere);
  Serial.print("Enviando: ");
  Serial.println(caractere);
  delay(1000); // Envia a cada 1 segundo
}

void enviarUART(char data) {
  byte parity = calcularParidade(data);

  unsigned long startTime = micros();

  // Envio do start bit
  digitalWrite(TX_PIN, LOW);
  waitForNextBit(&startTime);

  // Envio dos 8 bits de dados (LSB primeiro)
  for (int i = 0; i < 8; i++) {
    int bit = (data >> i) & 0x01;
    digitalWrite(TX_PIN, bit);
    waitForNextBit(&startTime);
  }

  // Envio do bit de paridade
  digitalWrite(TX_PIN, parity);
  waitForNextBit(&startTime);

  // Envio do stop bit
  digitalWrite(TX_PIN, HIGH);
  waitForNextBit(&startTime);
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