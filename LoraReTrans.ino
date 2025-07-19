#include <RadioLib.h>

// Configuração dos pinos (mesma pinagem dos seus arquivos)
#define LORA_CS      8   // GPIO8 (LORA_NSS)
#define LORA_DIO1    14  // GPIO14 (DIO1)
#define LORA_RST     12  // GPIO12 (LORA_RST)
#define LORA_BUSY    13  // GPIO13 (LORA_BUSY)

// Inicializa o rádio SX1262
SX1262 lora = new Module(LORA_CS, LORA_DIO1, LORA_RST, LORA_BUSY);
int cont;

// Variável para armazenar a última mensagem recebida
String ultimaMensagem;
unsigned long ultimaRecebida = 0;
const unsigned long timeoutRepeticao = 5000; // 5 segundos para evitar repetição em loop

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Iniciando Repetidor LoRa...");

  // Debug: exibe os pinos configurados
  Serial.print("LORA_CS: ");   Serial.println(LORA_CS);
  Serial.print("LORA_DIO1: "); Serial.println(LORA_DIO1);
  Serial.print("LORA_RST: ");  Serial.println(LORA_RST);
  Serial.print("LORA_BUSY: "); Serial.println(LORA_BUSY);

  // Reset manual do rádio
  pinMode(LORA_RST, OUTPUT);
  digitalWrite(LORA_RST, LOW);
  delay(50);
  digitalWrite(LORA_RST, HIGH);
  delay(200);

  // Inicializa o LoRa com mesma configuração do transmissor/receptor
  // (915 MHz, BW 62.5 kHz, SF11, CR4/7, sync word /*Endereco*/, 17 dBm)
  int state = lora.begin(915.0, 125.0, 11, 7, /*Endereco*/, 17, 8, 1.8); // Usar o mesmo endereco das outras LORAs
  
  if (state != RADIOLIB_ERR_NONE) {
    Serial.print("Falha ao iniciar LoRa! Código: ");
    Serial.println(state);
    while (1); // Trava em caso de erro
  }
  
  Serial.println("Repetidor LoRa inicializado com sucesso!");
  Serial.println("Aguardando mensagens para repetir...");
}

void loop() {
  // 1. Tentar receber mensagem
  String mensagem;
  int state = lora.receive(mensagem);
  
  if (state == RADIOLIB_ERR_NONE) {
    // Mensagem recebida com sucesso
    Serial.print("Mensagem recebida: ");
    Serial.println(mensagem);
    Serial.print("RSSI: ");
    Serial.println(lora.getRSSI());
    
    // Armazena a mensagem e o horário de recebimento
    ultimaMensagem = mensagem;
    ultimaRecebida = millis();
    
    // 2. Retransmitir a mensagem
    Serial.println("Retransmitindo mensagem...");
    int txState = lora.transmit(mensagem);
    
    if (txState == RADIOLIB_ERR_NONE) {
      Serial.println("Mensagem retransmitida com sucesso!");
    } else {
      Serial.print("Falha na retransmissão! Código: ");
      Serial.println(txState);
    }
    
  } /* Caso precise reenviar a ultima mensagem se perca de sinal
      else if (state == RADIOLIB_ERR_RX_TIMEOUT) {
    // Nenhuma mensagem recebida - verifica se precisa retransmitir mensagem antiga
    
    if (ultimaMensagem.length() > 0 && (millis() - ultimaRecebida) > timeoutRepeticao) {
      Serial.println("Nenhuma mensagem nova - repetindo a última após timeout");
      int txState = lora.transmit(ultimaMensagem);
      
      if (txState == RADIOLIB_ERR_NONE) {
        Serial.println("Mensagem repetida com sucesso!");
      } else {
        Serial.print("Falha na repetição! Código: ");
        Serial.println(txState);
      }
      
      ultimaRecebida = millis(); // Atualiza o tempo da última repetição
    }
  }*/ else {
    Serial.print("Erro na recepção: ");
    Serial.println(state);
  }
  
  delay(500); // Pequeno delay para evitar sobrecarga
}