//---------------------------------------Código destinado a fase de coleta de dados a empresa---------------------------------------
//                                                         Z&C Peluqueria


//BIBLIOTECAS-----------------------------------------------------------------------------------------------------------------------
#include <ssl_client.h>
#include <WiFiClientSecure.h>
#include <WiFiClient.h>
#include <WiFi.h>
#include "DHT.h"
#include <Wire.h>
#include <Adafruit_BMP085.h>
#include <Secrets.h>
#include <MICS6814.h>


//WiFiClientSecure client;//Cria um cliente seguro (para ter acesso ao HTTPS)
WiFiClient client;//Cria um cliente seguro (para ter acesso ao HTTPS)

String textFix = "GET /forms/d/e/1FAIpQLSfe9c11ac7_saB9AwhB7l19xrb8k66LK5KckP6aBdgTpJ31tw/formResponse?ifq";
//Essa String sera uma auxiliar contendo o link utilizado pelo GET, para nao precisar ficar re-escrevendo toda hora


//Condiguração dos Sensores---------------------------------------------------------------------------------------------------------
Adafruit_BMP085 bmp;
#define DHTPIN 4 // pino de dados do DHT11
#define DHTTYPE DHT11 // define o tipo de sensor, no caso DHT11

DHT dht(DHTPIN, DHTTYPE);

//Configuração das Variáveis--------------------------------------------------------------------------------------------------------

//Váriaveis auxiliares.
int Contador = 0;
int Quantidade = 0;
int Tempo_Total = 10; //700;

//DHT11
float Umidade_DHT11;
float Temperatura_DHT11;

//BMP180
float Temperatura_BMP180;
float Pressao_BMP180;
float Altitude_BMP180;

//MICS6814
#define PIN_CO   32   // CO
#define PIN_NO2  34  // NH3
#define PIN_NH3  35 // NO2

double CO2_MICS_6814;
double NH3_MICS_6814;
double NO2_MICS_6814;

double sensorValue1; //CO  //variável para armazenar o valor proveniente do sensor
double sensorValue2; //NO2
double sensorValue3; //NH3

MICS6814 gas(PIN_CO, PIN_NO2, PIN_NH3);

const char* ssid       = "Claudio Auto Eletrica";
const char* password   = "autoeletrica";

//Void SETUP---------------------------------------------------------------------------------------------------------
void setup() {



  pinMode(LED_BUILTIN, OUTPUT); //Configura o led da placa como saida
  digitalWrite(LED_BUILTIN, LOW); //Configura ele para iniciar desligado.

  Serial.begin(9600);//Inicia a comunicacao serial
  Serial.println("");
  dht.begin();
  bmp.begin();

  WiFi.mode(WIFI_STA);//Habilita o modo estaçao
  WiFi.begin(ssid, password);//Conecta na rede

  Serial.printf("Conectando na rede %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" Conectado!");

  delay(2000);//Espera um tempo para se conectar no WiFi

  //MICS6814
  Serial.println("Calibrando Sensor...");
  gas.calibrate();

  Serial.println("\nCalibração concluida!");

}

//Void LOOP---------------------------------------------------------------------------------------------------------
void loop() {





  //Request dos  Sensores---------------------------------------------------------------------------------------------
  //DHT11
  Umidade_DHT11 = dht.readHumidity();
  Temperatura_DHT11 = dht.readTemperature();

  //BMP180

  Temperatura_BMP180 = bmp.readTemperature();
  Pressao_BMP180 = bmp.readPressure();
  Altitude_BMP180 = bmp.readAltitude();

  //MICS6814
  for (int i = 0; i < 5; i ++) {
    sensorValue1 += gas.measure(CO); //variável para armazenar o valor proveniente do sensor
    sensorValue2 += gas.measure(NO2);
    sensorValue3 += gas.measure(NH3);
    delay(1000);
  }
  sensorValue1 /= 5;  //CO
  sensorValue2 /= 5;  //NO2
  sensorValue3 /= 5;  //NH3




  //Conexão com o servidor do Googgle----------------------------------------------------------------------------------
  //Tenta se conectar ao servidor do Google docs na porta 443 (HTTPS)
  if (client.connect("docs.google.com", 443) == 1) {
    Serial.println("Conectado.");//Mostra no monitor que foi conectado
    String toSend = textFix + "&entry.775614468=" + Temperatura_DHT11 + "&entry.1949165429=" + Umidade_DHT11 + "&entry.1206606471=" + Temperatura_BMP180 + "&entry.1810199323=" + Pressao_BMP180 + "&entry.1560309650=" + Altitude_BMP180 + "&entry.126476090=" + CO2_MICS_6814 + "&entry.1130407713=" + NH3_MICS_6814 + "&entry.139815761=" + NO2_MICS_6814; //Atribuimos a String auxiliar na nova String que sera enviada
    toSend += "&submit=Submit HTTP/1.1";//Completamos o metodo GET para nosso formulario.
    Serial.println("Dados enviados.");//Mostra no monitor que foi enviado
    client.println(toSend);//Enviamos o GET ao servidor-
    client.println("Host: docs.google.com");//-
    client.println();//-
    client.stop();//Encerramos a conexao com o servidor
    Serial.println("Dados enviados.");//Mostra no monitor que foi enviado
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000);
    digitalWrite(LED_BUILTIN, LOW);
    delay(1000);

  } else {
    Serial.println("Erro ao se conectar");//Se nao for possivel conectar no servidor, ira avisar no monitor.
    for (int i = 0; i < 5; i++) {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(100);
      digitalWrite(LED_BUILTIN, LOW);
      delay(100);
    }
  }




  //Finalização dos  Sensores----------------------------------------------------------------------------------------
  // Zera todas as váriaveis para que a leitura atual nao interfira na próxima.
  Contador = 0; //Zera a váriavel "Contador", para que no reiniciar o loop do-while a condição funcione.

  //DHT11
  Umidade_DHT11 = 0;
  Temperatura_DHT11 = 0;

  //BMP180
  Temperatura_BMP180 = 0;
  Pressao_BMP180 = 0;
  Altitude_BMP180 = 0;

  //MICS6814
  CO2_MICS_6814 = 0;  //CO
  NO2_MICS_6814 = 0;  //NO2
  NH3_MICS_6814 = 0;  //NH3
  sensorValue1 = 0;  //CO
  sensorValue2 = 0;  //NO2
  sensorValue3 = 0;  //NH3


  //Tempos entre uma gravação e outra-------------------------------------------------------------------------------
  delay(5000); // Espera 5 segundos para gravar a proxima String de dados.
  //delay(900000); // Espera 15 minutos para gravar a proxima String de dados.
}