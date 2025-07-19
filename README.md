# Sistema de Comunicação Industrial com LoRaWAN e CLP Allen-Bradley

Projeto completo para integração de sensores industriais com CLP Allen-Bradley Micro850 via comunicação LoRaWAN, utilizando placas ESP32-S3 LoRa V3 da Heltec.

## ⚙️ Componentes do Sistema
1. **Coletor de Dados** (`EnviaLoraLeituraADS.ino`)
   - Leitura de sensores via ADS1115 (16-bit ADC)
   - Transmissão Radio dos dados via LoRa (915MHz)
   
2. **Repetidor LoRa** (`LoraReTrans.ino`)
   - Retransmissão de pacotes para aumentar alcance
   - Sistema de failover com cache da última mensagem

3. **Gateway CLP** (`Recebe_e_Envia_RX-TX.ino`)
   - Recebimento de dados via LoRa
   - Interface serial com CLP Micro850 via RS485 (MAX485)

## 📡 Especificações Técnicas
| Parâmetro          | Valor               |
|--------------------|---------------------|
| Frequência LoRa    | 915 MHz             |
| Configuração LoRa  | SF11, BW 125 kHz    |
| Sensibilidade RX   | -137 dBm            |
| Potência TX        | 17 dBm              |
| Protocolo Serial   | RS485 (Modo Half-Duplex) |
| Velocidade Serial  | 9600 bps            |
| Interface CLP      | Micro850 Serial Port|

## ⚠️ Pré-requisitos
### Bibliotecas Arduino
"adafruit/Adafruit ADS1X15"
"jgromes/RadioLib"
