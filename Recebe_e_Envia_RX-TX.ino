#include <RadioLib.h>

// Configuração LoRa
#define LORA_CS      8
#define LORA_DIO1    14
#define LORA_RST     12
#define LORA_BUSY    13
SX1262 lora = new Module(LORA_CS, LORA_DIO1, LORA_RST, LORA_BUSY);

// Configuração UART para o CLP Micro850
// Conversor logico utilizado foi um MAX485
#define UART2_TX_PIN 20  // GPIO17 (UART2_TX)
#define UART2_RX_PIN 19  // GPIO16 (UART2_RX)
#define RS485_DE_PIN 4   // GPIO4 para controle DE/RE do MAX485

const float sensorMax = 24050.0;
const float sensorMin = 0.0;

void setup() {
  Serial.begin(115200); // Debug via USB
  Serial1.begin(9600, SERIAL_8N1, UART2_RX_PIN, UART2_TX_PIN); // UART2 para o CLP
  pinMode(RS485_DE_PIN, OUTPUT);
  digitalWrite(RS485_DE_PIN, LOW); // Inicia em modo recepção

  // Inicializa LoRa
  int state = lora.begin(915.0, 125.0, 11, 7, /*Endereco*/, 17, 8, 1.8); // O endereco deve ser o mesmo das demais LORAs
  if (state != RADIOLIB_ERR_NONE) {
    Serial.println("Falha ao iniciar LoRa!");
    while (1);
  }
  Serial.println("Sistema pronto - aguardando dados LoRa...");
}

void loop() {
  String mensagem;
  int state = lora.receive(mensagem);
  
  if (state == RADIOLIB_ERR_NONE) {
    float sensorValue = mensagem.toFloat();
    if (sensorValue != 0.0 || mensagem.equals("0") || mensagem.equals("0.0")) {
      processSensorValue(sensorValue); // Processa e envia para o CLP
      
    }
  }
}

void processSensorValue(float sensorValue) {
  // Verifica se informação é valida para envio
  if (sensorValue < sensorMin || sensorValue > sensorMax) {
    Serial.println("Valor fora da faixa!");
    Serial1.print("Valor fora da faixa!");
    return;
  }

  // Formata o valor para o CLP 
  /* Para que funcione corretamente eh necessario enviar os bits de terminacao, 0x0D 0x0A ou \r\n,
  fazendo isso o CLP sabera quando a mensagem cheou ao fim, evitando o bufferoverflow da entrada serial */
  String clpMsg = String(sensorValue); 

  // Envia via RS-485 
  digitalWrite(RS485_DE_PIN, HIGH); // Habilita o envio
  Serial1.print(clpMsg);
  Serial1.print("\r\n");
  delay(500);
  digitalWrite(RS485_DE_PIN, LOW);

  Serial.print("Enviado para CLP: ");
  Serial.println(clpMsg);
  delay(3000); //Teste
}