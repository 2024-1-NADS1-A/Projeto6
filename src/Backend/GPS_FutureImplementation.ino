#include <TinyGPSPlus.h>
#include <HardwareSerial.h>

int RXPin = 2; // Pino onde você conectou o TX do módulo GPS
int TXPin = 3; // Pino onde você conectou o RX do módulo GPS
int GPSBaud = 115200;

TinyGPSPlus gps;
HardwareSerial gpsSerial(1); // Use a Serial1 (UART1) no ESP32

unsigned long lastGpsCheckTime = 0; // Variável para controlar o intervalo de tempo
const unsigned long gpsCheckInterval = 120000; // Intervalo de 120 segundos (em milissegundos)

void setup()
{
  Serial.begin(115200);
  gpsSerial.begin(GPSBaud, SERIAL_8N1, RXPin, TXPin);
}

void loop()
{
  unsigned long currentTime = millis();

  // Verifica se o intervalo de tempo (120 segundos) foi atingido
  if (currentTime - lastGpsCheckTime >= gpsCheckInterval)
  {
    lastGpsCheckTime = currentTime; // Atualiza o tempo da última verificação

    // Toda vez que for lida uma nova sentença NMEA, chamaremos a função displayInfo() para mostrar os dados na tela
    while (gpsSerial.available() > 0)
      if (gps.encode(gpsSerial.read()))
        displayInfo();
  }

  // Se em 5 segundos não for detectada nenhuma nova leitura pelo módulo, será mostrada esta mensagem de erro.
  if (currentTime - lastGpsCheckTime >= 5000 && gps.charsProcessed() < 10)
  {
    Serial.println("Sinal GPS não detectado");
    // Você pode adicionar outras ações aqui, se necessário.
  }
}

void displayInfo()
{
  // Resto do seu código...
}
