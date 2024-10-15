# Comunicação UART Personalizada entre Arduinos

Este projeto demonstra como implementar uma comunicação serial UART personalizada entre dois Arduinos utilizando pinos digitais genéricos, sem utilizar os pinos `tx` e `rx` integrados. A comunicação é realizada através de **bit banging**, respeitando o protocolo UART com 1 bit de paridade, 1 bit de start, 1 bit de stop e um baud rate de **9600 bits/s**.

## Índice

- [Objetivo](#objetivo)
- [Materiais Necessários](#materiais-necessários)
- [Configuração de Hardware](#configuração-de-hardware)
- [Explicação dos Códigos](#explicação-dos-códigos)
  - [Código do Transmissor](#código-do-transmissor)
  - [Código do Receptor](#código-do-receptor)
- [Conceitos Utilizados](#conceitos-utilizados)
  - [Framing UART](#framing-uart)
  - [Operadores Bitwise](#operadores-bitwise)
  - [Controle de Tempo sem `delay()`](#controle-de-tempo-sem-delay)
- [Execução do Projeto](#execução-do-projeto)
- [Monitoramento com Analog Discovery](#monitoramento-com-analog-discovery)
- [Conclusão](#conclusão)
- [Referências](#referências)

## Objetivo

Implementar a transmissão de um caractere entre dois Arduinos utilizando uma comunicação UART personalizada em pinos digitais genéricos, sem usar os pinos `tx` e `rx` integrados. O projeto deve:

- Respeitar o framing UART (1 start bit, 8 data bits, 1 parity bit, 1 stop bit).
- Operar a um baud rate de **9600 bits/s**.
- Identificar erros de paridade no receptor.
- Não utilizar funções `delay()` ou `delayMicroseconds()` para controle de tempo.
- Possibilitar o monitoramento dos sinais com um **Analog Discovery**.

## Materiais Necessários

- 2 Arduinos (Uno, Mega ou compatíveis)
- Cabos jumper para conexões
- Computador com Arduino IDE instalado
- **Opcional**: Analog Discovery ou osciloscópio para monitoramento dos sinais

## Configuração de Hardware

### Conexões

1. **Transmissor Arduino**:
   - Pino de transmissão (`TX_PIN`): **Pino 2**
2. **Receptor Arduino**:
   - Pino de recepção (`RX_PIN`): **Pino 3**

### Diagrama de Conexão

[Arduino Transmissor] [Arduino Receptor] TX_PIN (Pino 2) ----------- RX_PIN (Pino 3) GND -------------------- GND


- Conecte o **pino 2** do Arduino transmissor ao **pino 3** do Arduino receptor.
- Conecte os **GNDs** de ambos os Arduinos para estabelecer um referencial comum.

## Explicação dos Códigos

### Código do Transmissor

#### Arquivo: `transmissor.ino`

**Explicação Detalhada**

- **Definições Iniciais**:
  - `TX_PIN`: Pino utilizado para transmissão.
  - `BAUD_RATE`: Taxa de transmissão em bits por segundo.
  - `BIT_DURATION`: Duração de cada bit em microssegundos, calculada como `1.000.000 / BAUD_RATE`.

- **Função `setup()`**:
  - Configura o `TX_PIN` como saída.
  - Define o estado inicial da linha como **HIGH** (nível alto), indicando linha ociosa.

- **Função `loop()`**:
  - Define o caractere a ser enviado (neste caso, `'A'`).
  - Chama a função `enviarUART(data)` para transmitir o caractere.
  - Aguarda 1 segundo antes de enviar o próximo caractere.

- **Função `enviarUART(char data)`**:
  - **Cálculo da Paridade**:
    - Chama `calcularParidade(data)` para obter o bit de paridade par.
  - **Controle de Tempo**:
    - `startTime` captura o tempo inicial da transmissão usando `micros()`.
  - **Envio do Start Bit**:
    - Coloca a linha em nível baixo (`LOW`).
    - Chama `waitForNextBit(&startTime)` para esperar a duração de um bit.
  - **Envio dos Bits de Dados**:
    - Loop de 8 iterações para enviar cada bit do byte de dados.
    - Usa `(data >> i) & 0x01` para extrair o i-ésimo bit.
    - Atualiza o pino `TX_PIN` com o valor do bit.
    - Chama `waitForNextBit(&startTime)` após cada bit.
  - **Envio do Bit de Paridade**:
    - Transmite o bit de paridade calculado.
    - Chama `waitForNextBit(&startTime)`.
  - **Envio do Stop Bit**:
    - Coloca a linha em nível alto (`HIGH`).
    - Chama `waitForNextBit(&startTime)`.

- **Função `waitForNextBit(unsigned long* previousTime)`**:
  - Aguarda até que o tempo de duração de um bit tenha passado desde `previousTime`.
  - Atualiza `previousTime` adicionando `BIT_DURATION`.

- **Função `calcularParidade(byte data)`**:
  - Inicializa a paridade em 0.
  - Loop para calcular a paridade XOR dos bits do byte.
  - Retorna a paridade calculada (paridade par).

### Código do Receptor

#### Arquivo: `receptor.ino`

**Explicação Detalhada**

- **Definições Iniciais**:
  - `RX_PIN`: Pino utilizado para recepção.
  - `BAUD_RATE`: Taxa de transmissão em bits por segundo.
  - `BIT_DURATION`: Duração de cada bit em microssegundos.

- **Função `setup()`**:
  - Configura o `RX_PIN` como entrada.
  - Inicia a comunicação serial com o computador a 9600 baud para exibir os dados recebidos.

- **Função `loop()`**:
  - Verifica se o `RX_PIN` está em nível baixo, indicando o início de uma transmissão (start bit).
  - Se uma transmissão está ocorrendo, chama `receberUART()` para ler o caractere.
  - Se um caractere válido foi recebido, imprime no monitor serial.

- **Função `receberUART()`**:
  - **Sincronização Inicial**:
    - Captura `startTime` usando `micros()`.
    - Chama `waitForHalfBitTime(&startTime)` para aguardar meio período de bit, posicionando a leitura no meio do start bit.
    - Verifica se o pino ainda está em nível baixo. Se não, retorna `0` indicando erro de sincronização.
  - **Leitura dos Bits de Dados**:
    - Avança para o próximo período de bit com `waitForNextBit(&startTime)`.
    - Loop de 8 iterações para ler cada bit de dados.
    - Lê o valor do pino `RX_PIN`.
    - Usa `data |= (bit << i)` para montar o byte recebido.
    - Chama `waitForNextBit(&startTime)` após cada leitura.
  - **Leitura do Bit de Paridade**:
    - Lê o bit de paridade.
    - Chama `waitForNextBit(&startTime)`.
  - **Leitura do Stop Bit**:
    - Lê o stop bit e verifica se está em nível alto.
    - Se não estiver em nível alto, imprime "Erro no stop bit!" e retorna `0`.
  - **Verificação da Paridade**:
    - Calcula a paridade dos dados recebidos com `calcularParidade(data)`.
    - Compara com o `parityBit` recebido.
    - Se não corresponder, imprime "Erro de paridade!" e retorna `0`.
  - Retorna o `data` recebido se não houver erros.

- **Funções de Controle de Tempo**:
  - **`waitForHalfBitTime(unsigned long* previousTime)`**:
    - Aguarda meio período de bit para posicionar a leitura no meio do start bit.
    - Atualiza `previousTime` adicionando `BIT_DURATION / 2`.
  - **`waitForNextBit(unsigned long* previousTime)`**:
    - Aguarda até que o tempo de um bit tenha decorrido.
    - Atualiza `previousTime` adicionando `BIT_DURATION`.

- **Função `calcularParidade(byte data)`**:
  - Calcula a paridade par dos bits de dados recebidos.

## Conceitos Utilizados

### Framing UART

O protocolo UART define um quadro (frame) de transmissão composto por:

- **Start Bit**: Indica o início da transmissão (nível baixo).
- **Data Bits**: Geralmente 8 bits, transmitidos do LSB ao MSB.
- **Parity Bit**: Opcional, utilizado para detecção de erros.
- **Stop Bit**: Indica o fim da transmissão (nível alto).

Neste projeto, utilizamos:

- 1 start bit
- 8 data bits
- 1 bit de paridade (paridade par)
- 1 stop bit

### Operadores Bitwise

Os operadores bit a bit são fundamentais para manipular os bits individuais dos dados:

- **`<<` (Left Shift)**: Desloca os bits para a esquerda.
  - Exemplo: `data << 1` desloca todos os bits de `data` uma posição à esquerda.
- **`>>` (Right Shift)**: Desloca os bits para a direita.
  - Exemplo: `data >> 1` desloca todos os bits de `data` uma posição à direita.
- **`&` (AND Bitwise)**: Realiza a operação AND bit a bit.
  - Exemplo: `data & 0x01` isola o bit menos significativo.
- **`|=` (OR Bitwise Composto)**: Atribui o resultado da operação OR bit a bit.
  - Exemplo: `data |= (bit << i)` configura o i-ésimo bit de `data`.

### Controle de Tempo sem `delay()`

Em vez de utilizar `delay()` ou `delayMicroseconds()`, o controle de tempo é feito utilizando a função `micros()`:

- **`micros()`**: Retorna o número de microssegundos desde que o Arduino começou a executar o programa.
- **Controle de Tempo Preciso**: Permite aguardar até que um determinado período tenha passado sem bloquear o microcontrolador, possibilitando a execução de outras tarefas se necessário.

As funções `waitForNextBit()` e `waitForHalfBitTime()` utilizam `micros()` para controlar precisamente o timing entre os bits transmitidos e recebidos.

## Execução do Projeto

1. **Montagem do Circuito**:
   - Conecte os Arduinos conforme o diagrama de conexão.
   - Certifique-se de que os GNDs estejam conectados.

2. **Carregamento dos Códigos**:
   - Abra o Arduino IDE.
   - Carregue o código `transmissor.ino` no Arduino transmissor.
   - Carregue o código `receptor.ino` no Arduino receptor.

3. **Monitoramento da Comunicação**:
   - No Arduino receptor, abra o Monitor Serial (defina a taxa em 9600 baud).
   - Você deverá ver a mensagem:
     ```
     Caractere recebido: A
     ```
     sendo exibida a cada segundo.

4. **Teste de Erro de Paridade**:
   - Para verificar a detecção de erro de paridade, altere propositalmente um bit dos dados ou da paridade no transmissor.
   - No código do transmissor, após calcular a paridade, você pode inverter o bit de paridade:
     ```cpp
     parity = !parity; // Inverte o bit de paridade para simular erro
     ```
   - Carregue o código alterado e observe que o receptor exibirá "Erro de paridade!".

## Monitoramento com Analog Discovery

Para uma análise mais aprofundada e para cumprir os requisitos de monitoramento, siga os passos:

1. **Conexão do Instrumento**:
   - Conecte as sondas digitais do Analog Discovery aos pinos `TX_PIN` e `RX_PIN`.
   - Conecte o GND do instrumento ao GND dos Arduinos.

2. **Configuração do Software**:
   - Abra o software WaveForms.
   - Configure os canais digitais para monitorar os pinos conectados.
   - Defina o baud rate para 9600 bits/s.

3. **Captura dos Sinais**:
   - Inicie a captura para visualizar os sinais digitais.
   - Utilize a ferramenta de decodificação UART para interpretar os sinais.

4. **Análise dos Resultados**:
   - Verifique se os sinais correspondem ao protocolo UART esperado.
   - Observe os tempos de transição e os níveis lógicos.

## Conclusão

Este projeto demonstra como implementar uma comunicação UART personalizada entre dois Arduinos utilizando pinos digitais genéricos e técnicas de **bit banging**. Além de cumprir os requisitos propostos, o projeto reforça conceitos importantes como manipulação de bits, controle de tempo preciso sem bloqueio do microcontrolador e compreensão do protocolo UART.

A eliminação das funções `delay()` e `delayMicroseconds()` torna o código mais eficiente e responsivo, permitindo futuras expansões e a integração com sistemas mais complexos.

## Referências

- **Arduino Reference**:
  - [Digital I/O](https://www.arduino.cc/reference/en/language/functions/digital-io/)
  - [micros()](https://www.arduino.cc/reference/en/language/functions/time/micros/)
- **Operadores Bitwise em C/C++**:
  - [Bitwise Operators - Cprogramming.com](https://www.cprogramming.com/tutorial/bitwise_operators.html)
- **Protocolo UART**:
  - [UART Communication - SparkFun](https://learn.sparkfun.com/tutorials/serial-communication)
- **Monitoramento com Analog Discovery**:
  - [Using the Protocol Analyzer - Digilent](https://reference.digilentinc.com/learn/instrumentation/tutorials/protocol-analyzer/start)

---

**Nota**: Este projeto é uma excelente oportunidade para aprofundar conhecimentos em sistemas embarcados, protocolos de comunicação e técnicas de programação de baixo nível. Sinta-se à vontade para expandir o código, adicionar novos recursos ou adaptá-lo para diferentes aplicações.
