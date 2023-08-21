/******************************************************************************************
NOMBRE: 
  DHT20_WiFi.ino 
AUTOR:
  Juan Esteban Gomez
FECHA DE CREACIÓN:
  01/08/2023
FECHA DE ACTUALIZACIÓN:
  01/08/2023
FUNCIONES:
  Se encarga de tomar datos de un sensor de humedad y temperatura TH20, muestra la info
  por una pantalla OLED incluida en un TTGO LoRa32.
PENDIENTES:
  
IDEAS PARA EL FUTURO:
  
******************************************************************************************/

/*** INCLUDES ****************************************************************************/
  /*** LIBRERIAS ***/
  #include <Arduino.h>
  #include <TFT_eSPI.h>
  #include <SPI.h>
  #include "WiFi.h"
  #include <Wire.h>
  #include "DHT20.h"
  #include "UbidotsEsp32Mqtt.h"
  /*** LIBRERIAS PROPIAS ***/
/*****************************************************************************************/

/*** DEFINICIONES DE LAS FUNCIONES *******************************************************/
  void callback(char *topic, byte *payload, unsigned int length){
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (int i = 0; i < length; i++)
    {
      Serial.print((char)payload[i]);
    }
    Serial.println();
  }
/*****************************************************************************************/

/*** DEFINICIÓN DE VARIABLES *************************************************************/
  DHT20 DHT(&Wire);

  const char *UBIDOTS_TOKEN = "BBFF-s28LKWunSHYGoqw0NYf1JaxfZRe0vq";  // Put here your Ubidots TOKEN
  const char *WIFI_SSID = "VIRUS1801";      // Put here your Wi-Fi SSID
  const char *WIFI_PASS = "52029450";      // Put here your Wi-Fi password
  const char *DEVICE_LABEL = "Sensor";   // Put here your Device label to which data  will be published

  Ubidots ubidots(UBIDOTS_TOKEN);

  TFT_eSPI tft = TFT_eSPI();
  float Humedad = 0;
  float Temperatura = 0;
  char WifiCon = 0;

  //*** VARIABLES DE TIEMPO ***/
  unsigned long TS_ant, TS_act; //Tiempos para el muestreo del sensor
  const long TS_fin = 1000/40; // Tiempo en milisegundos para un tiempo de muestreo de 40 Hz
  unsigned long TP_ant, TP_act; //Tiempos para la muestra del dato en el OLED
  const long TP_fin = 500; // Tiempo en milisegundos para una actualizacion de 2 Hz
  unsigned long Tx_ant, Tx_act; //Tiempos para la muestra del dato en el OLED
  const long Tx_fin = 5000; // Tiempo en milisegundos para una actualizacion de 0.2 Hz


/*****************************************************************************************/

/*** INICIALIZACIÓN **********************************************************************/
  void setup() {
  // put your setup code here, to run once:
  DHT.begin();  //  ESP32 default pins 21 22
  Serial.begin(115200);
  tft.init();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);
  tft.drawString("Realizado por:", 10, 5, 2);
  tft.drawString("Juan E. Gomez", 10, 23, 4);
  tft.drawString("Humedad:", 10, 70, 2);
  tft.drawString("Temperatura", 140, 70, 2);
  tft.drawFastHLine(10, 50, 170, TFT_GREEN);
  tft.fillRect(110, 65, 3, 80, TFT_GREEN);

  ubidots.connectToWifi(WIFI_SSID, WIFI_PASS);
  ubidots.setCallback(callback);
  ubidots.setup();
  ubidots.reconnect();

  delay(1000);

  Serial.println("Humidity, Temperature");

  TS_ant = millis();
  TP_ant = millis();
  Tx_ant = millis();
  }
/*****************************************************************************************/

/*** PROGRAMA PRINCIPAL ******************************************************************/
  void loop() {
  TS_act = millis();
  TP_act = millis();
  Tx_act = millis();

  // Lectura del sensor
  if (TS_act - TS_ant >= TS_fin) {
    TS_ant = TS_act;
    // Se toma el dato
    DHT.read();
    Humedad = DHT.getHumidity();
    Temperatura = DHT.getTemperature();
  }

  // Pantalla
  if (TP_act - TP_ant >= TP_fin) {
    TP_ant = TP_act;
    // Se muestra el dato en la pantalla
    if (WifiCon = 1){
      tft.drawString("WiFi", 200, 5, 2);
    }
    tft.drawString((String(Humedad,2))+'%', 10, 100, 4);
    tft.drawString((String(Temperatura,2))+'C', 140, 100, 4);
    Serial.print(Humedad);
    Serial.print("% \t");
    Serial.print(Temperatura);
    Serial.println("C");
  }

  // Transmision
  if (Tx_act - Tx_ant >= Tx_fin) {
    Tx_ant = Tx_act;
    // Se envia el dato a UbiDots
    WifiCon = 1;
    if (WiFi.status() != WL_CONNECTED){
      WifiCon = 0;
    }
    if (!ubidots.connected()){
      ubidots.reconnect();
    }
    ubidots.add("Humedad", Humedad);
    ubidots.add("Temperatura", Temperatura);
    ubidots.publish(DEVICE_LABEL);
    ubidots.loop();
  }

}
/*****************************************************************************************/
