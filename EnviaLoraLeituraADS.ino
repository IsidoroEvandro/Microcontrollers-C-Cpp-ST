#include <RadioLib.h>
#include <Wire.h>
#include <Adafruit_ADS1X15.h>

// Configuração dos pinos LoRa
#define LORA_CS      8   // GPIO8 (LORA_NSS)
#define LORA_DIO1    14  // GPIO14 (DIO1)
#define LORA_RST     12  // GPIO12 (LORA_RST)
#define LORA_BUSY    13  // GPIO13 (LORA_BUSY)

// Objetos
SX1262 lora = new Module(LORA_CS, LORA_DIO1, LORA_RST, LORA_BUSY);
Adafruit_ADS1X15 ads;

// Variáveis para leitura do sensor
const float VREF = 3.300;       // Tensão de referência
const int ADC_MAX = 23428;    // Valor máximo para ADS1115; o valor maximo é de 24045 (Nosso sistema tem um sensor 24Vcc 0 a 20 mA, com shunt de 220ohms entre A0 e GND)
const int SEND_INTERVAL = 2000; // Intervalo entre envios (ms)
const int I2C_RETRIES = 3;    // Tentativas de leitura I2C

// Pinos I2C (ajuste para sua placa)
#define I2C_SDA 19
#define I2C_SCL 20

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  // Inicializa I2C com pinos explícitos
  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(100000); // Clock de 100kHz
  
  // Inicializa o ADS1115 com tratamento de erro
  bool adsInitialized = false;
  for(int i = 0; i < 5; i++) {
    if(ads.begin(0x48)) {
      adsInitialized = true;
      break;
    }
    Serial.println("Tentando conectar ao ADS1115...");
    delay(500);
  }
  
  if(!adsInitialized) {
    Serial.println("Falha ao inicializar o ADS1115! Verifique conexões I2C.");
    while(1);
  }
  
  ads.setGain(GAIN_TWOTHIRDS); // Ajuste conforme necessário
  
  // Inicializa o LoRa
  setupLora();
  
  Serial.println("Sistema pronto - iniciando leituras e transmissões...");
}

void loop() {
  // Faz a leitura do ADS1115 com tratamento de erro
  int16_t adcValue = 0;
  float voltage = 0.0;
  bool readSuccess = false;
  
  for(int i = 0; i < I2C_RETRIES; i++) {
    if(readADS(&adcValue, &voltage)) {
      readSuccess = true;
      break;
    }
    delay(100);
  }
  
  if(!readSuccess) {
    Serial.println("Falha na leitura do ADS1115!");
    delay(SEND_INTERVAL);
    return;
  }
  
  // Exibe no serial
  Serial.print("ADC: ");
  Serial.print(adcValue);
  Serial.print(" | Tensão: ");
  Serial.print(voltage, 4);
  Serial.println("V");
  
  // Prepara e envia a mensagem via LoRa
  String message = String(adcValue) + "|" + String(voltage, 4);
  sendLoraMessage(message);
  
  delay(SEND_INTERVAL);
}

bool readADS(int16_t *adcValue, float *voltage) {
  Wire.beginTransmission(0x48);
  if(Wire.endTransmission() != 0) {
    return false;
  }
  
  *adcValue = ads.readADC_SingleEnded(0);
  if(*adcValue == 0xFFFF) { // Valor de erro
    return false;
  }
  
  *voltage = (*adcValue * VREF) / ADC_MAX;
  return true;
}

void setupLora() {
  Serial.println("Inicializando módulo LoRa...");
  
  // Reset manual do rádio
  pinMode(LORA_RST, OUTPUT);
  digitalWrite(LORA_RST, LOW);
  delay(150);
  digitalWrite(LORA_RST, HIGH);
  delay(200);

  // Inicializa o LoRa (915 MHz, SF9, BW125 kHz)
  int state = lora.begin(915.0, 125.0, 11, 7, /*Endereco*/, 17, 8, 1.8); // O endereco deve ser o mesmo entre as LORAs. Ajuste com valor valido
  
  if (state != RADIOLIB_ERR_NONE) {
    Serial.print("Falha ao iniciar LoRa! Código: ");
    Serial.println(state);
    while (1);
  }
  Serial.println("LoRa inicializado com sucesso!");
}

void sendLoraMessage(String message) {
  Serial.print("Enviando: ");
  Serial.println(message);
  
  int state = lora.transmit(message);
  
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println("Mensagem enviada com sucesso!");
    Serial.print("RSSI: ");
    Serial.println(lora.getRSSI());
  } else {
    Serial.print("Falha no envio! Código: ");
    Serial.println(state);
  }
}