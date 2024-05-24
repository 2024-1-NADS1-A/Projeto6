// Definições do Blynk (devem ser definidas antes de incluir as bibliotecas)
#define BLYNK_TEMPLATE_ID "TMPL2vsilmRLW"
#define BLYNK_TEMPLATE_NAME "Quickstart Template"
#define BLYNK_AUTH_TOKEN "8_YjHIH1iab4o2l3hAD4znpiI8SJTpsE"

// Comente isso para desativar prints e economizar espaço
#define BLYNK_PRINT Serial

// Inclusão das bibliotecas necessárias
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// Suas credenciais WiFi. Defina a senha como "" para redes abertas.
char ssid[] = "Pedro";
char pass[] = "pague_agora";


#include <OneWire.h>
#include <DallasTemperature.h>

// GPIO onde o DS18B20 está conectado
const int oneWireBus = 25;

// Setup de uma instância oneWire para comunicar com dispositivos OneWire
OneWire oneWire(oneWireBus);

// Passar a referência oneWire para o sensor de temperatura Dallas
DallasTemperature sensors(&oneWire);

#define TdsSensorPin 27
#define VREF 3.3              // referência de voltagem analógica(Volt) do ADC
#define SCOUNT  30            // soma de pontos de amostra

int analogBuffer[SCOUNT];     // armazena o valor analógico no array, lido do ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0;
int copyIndex = 0;

float averageVoltage = 0;
float tdsValue = 0;
float temperature = 25;       // temperatura atual para compensação

// algoritmo de filtro mediano
int getMedianNum(int bArray[], int iFilterLen){
  int bTab[iFilterLen];
  for (byte i = 0; i<iFilterLen; i++)
  bTab[i] = bArray[i];
  int i, j, bTemp;
  for (j = 0; j < iFilterLen - 1; j++) {
    for (i = 0; i < iFilterLen - j - 1; i++) {
      if (bTab[i] > bTab[i + 1]) {
        bTemp = bTab[i];
        bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
      }
    }
  }
  if ((iFilterLen & 1) > 0){
    bTemp = bTab[(iFilterLen - 1) / 2];
  }
  else {
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
  }
  return bTemp;
}

void setup() {
  Serial.begin(115200); // Inicializa a comunicação serial a 115200 bps
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass); // Conecta ao Blynk usando as credenciais fornecidas
  pinMode(TdsSensorPin, INPUT);
  sensors.begin();
}

void loop(){
  Blynk.run(); // Mantém a conexão com o Blynk
  static unsigned long analogSampleTimepoint = millis();
  if(millis()-analogSampleTimepoint > 40U){     // a cada 40 milissegundos, leia o valor analógico do ADC
    analogSampleTimepoint = millis();
    analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin);    // leia o valor analógico e armazene no buffer
    analogBufferIndex++;
    if(analogBufferIndex == SCOUNT){ 
      analogBufferIndex = 0;
    }
  }   
  
  static unsigned long printTimepoint = millis();
  if(millis()-printTimepoint > 800U){
    printTimepoint = millis();
    for(copyIndex=0; copyIndex<SCOUNT; copyIndex++){
      analogBufferTemp[copyIndex] = analogBuffer[copyIndex];
      
      // leia o valor analógico de forma mais estável pelo algoritmo de filtragem mediana e converta para valor de voltagem
      averageVoltage = getMedianNum(analogBufferTemp,SCOUNT) * (float)VREF / 4096.0;
      
      // fórmula de compensação de temperatura: fResultadoFinal(25^C) = fResultadoFinal(atual)/(1.0+0.02*(fTP-25.0)); 
      float coeficienteDeCompensacao = 1.0+0.02*(temperature-25.0);
      // compensação de temperatura
      float voltagemCompensada = averageVoltage / coeficienteDeCompensacao;
      
      // converta o valor de voltagem para valor de TDS
      tdsValue=(133.42*voltagemCompensada*voltagemCompensada*voltagemCompensada - 255.86*voltagemCompensada*voltagemCompensada + 857.39*voltagemCompensada)*0.5;
      
      // Imprima o valor de TDS
      Serial.print("TDS Value: ");
      Serial.print(tdsValue, 0);
      Serial.println(" ppm");

      // Solicita as temperaturas e as imprime
      sensors.requestTemperatures(); 
      float temperatureC = sensors.getTempCByIndex(0);
      float temperatureF = sensors.getTempFByIndex(0);
      Serial.print("Temperature: ");
      Serial.print(temperatureC);
      Serial.print("°C, ");
      Serial.print(temperatureF);
      Serial.println("°F");


      Blynk.virtualWrite(V4, temperatureC); // Envia a frequência cardíaca ao Blynk
  Blynk.virtualWrite(V5, tdsValue); // Envia a velocidade ao Blynk
    }
  }
}
