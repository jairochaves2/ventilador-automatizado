#include "DHT.h"
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

#define BLYNK_PRINT Serial

//thingSpeak
String apiKey = "9ZKZJRLLTTD635B8";     //  <-- seu Write API key do site ThingSpeak
const char *ssidTS =  "REDEIOT";     // <-- substitua com o ssid e senha da rede Wifi
const char *passTS =  "iotnet18";
const char* server = "api.thingspeak.com";
WiFiClient client;

#define pinRele1 D0
#define pinSom D2
#define tempoMaximoDeUmaPalma  150 //milisegundos
#define tempoMaximoEntrePalmas 500 //milisegundos

//Token do Blynk
char auth[] = "bb395608a61c4ce591bb2ece7d2da5c0";

//Senhas Blink
char ssid[] = "REDEIOT";
char pass[] = "iotnet18";


int contaPalmas = 0;
long tempoEspera = 0;

long tempoInicial = 0;
long tempoFinal = 0;
//blynk
long tInicialB = 0;
long tFinalB = 0;

//blynkNotificação
long tInicialBN = 0;
long tFinalBN = 0;

//Declaração das variaveis que vão receber a temperatura e a umidade
float umidade;
float temperatura;

long tempoEsperaEntrePalmas = 0;
bool somLigado =false;
void executarAcao();

DHT dht(D1, DHT11);
void setup() {
  Serial.begin(9600);
  dht.begin();
  Blynk.begin(auth, ssid, pass);

  //Conectando ao Wifi para o ThingSpeak
  Serial.println("Conectando a ");
  Serial.println(ssidTS);
 
 
  WiFi.begin(ssidTS, passTS);
 
  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Conectado ao Wifi");
  
  pinMode(pinSom, INPUT);
  pinMode(pinRele1, OUTPUT);
  pinMode(D4, OUTPUT);
  digitalWrite(pinRele1,HIGH);
  digitalWrite(D4,LOW);
  
  //inicia os contadores de tempo
  tempoInicial=millis();
  tInicialB=millis();
  tInicialBN=millis();
}

void loop() {
//verifica o estado do sensor de som (ele fica normalmente com a porta ligada. Quando ouver uma palma, ele desliga momentaneamente a porta)
      
  int sensorSom = digitalRead(pinSom);
  
  //se o sensor detectou palmas
  if (sensorSom == LOW) {

     //espera um tempo para nao detectar a mesma palma mais de uma vez Blynk.run();
     if (tempoEspera == 0) {
        tempoEspera = tempoEsperaEntrePalmas = millis(); 
        contaPalmas++;
     } else if ((millis() - tempoEspera) >= tempoMaximoDeUmaPalma) {
        tempoEspera = 0;
     }
  }

  //caso exceda o tempo maximo entre palmas, zera o contador de palmas
  if ( (contaPalmas != 0) && ((millis() - tempoEsperaEntrePalmas) > 500) ) {
     executarAcao();
     contaPalmas = 0;
     tempoEsperaEntrePalmas = millis();
  }

  //tempo para fazer uma leitura do tempo
  tempoFinal=millis();
  if((tempoFinal-tempoInicial)>=20000 && somLigado==false){
    
    //Leitura de umidade
    umidade = dht.readHumidity();
    //Leitura de temperatura
    temperatura = dht.readTemperature();
    if(temperatura>34.00){
      digitalWrite(pinRele1,LOW);
    }if(isnan(temperatura)){
      Serial.println("Erro na Leitura do Sensor!");
    }else{
      digitalWrite(pinRele1,HIGH);
      
    }

    
    //A cada 1 hora é feito um envio de notificação caso a humidade do ar esteja baixa
    tFinalBN=millis();
    //if((tFinalBN-tInicialBN)>=(60000*60)){
    if((tFinalBN-tInicialBN)>=(40000)){  
      tInicialBN=millis();
      //Verifica se a umidade relativa do ar está abaixo de 40% se estiver, manda uma notificação para o celular
      if(umidade<=40.00){
        Blynk.notify("Atenção: Beba línquidos, Umidade do ar baixa");
      }
      
      //Envia os dados ao thingSpeak a cada 1 hora
      if (client.connect(server,80)){   //   "184.106.153.149" or api.thingspeak.com  
      
        String postStr = apiKey;
        postStr +="&field1="; //<-- atenção, esse é o campo 1 que você escolheu no canal do ThingSpeak
        postStr += String(temperatura);
        postStr +="&field2=";
        postStr += String(umidade);
        postStr += "\r\n\r\n";
        
        client.print("POST /update HTTP/1.1\n");
        client.print("Host: api.thingspeak.com\n");
        client.print("Connection: close\n");
        client.print("X-THINGSPEAKAPIKEY: "+apiKey+"\n");
        client.print("Content-Type: application/x-www-form-urlencoded\n");
        client.print("Content-Length: ");
        client.print(postStr.length());
        client.print("\n\n");
        client.print(postStr);
      }
      client.stop();
    }
    
      
    Serial.print("Temperatura: ");
    Serial.println(temperatura);
    Serial.print("Umidade: ");
    Serial.println(umidade);
    Serial.println();

    
    tempoInicial=millis();
  }
  
  // Faz uma atualização no Blynk a cada 1 segundo e meio
  tFinalB=millis();
  if((tFinalB-tInicialB)>=1500){
    Blynk.run();
    tInicialB=millis();  
  }
  //Pino reservado para o controle de uso da temperatura pelo Blynk
  if(digitalRead(D4)==HIGH){
       somLigado=false;
    }else{
      somLigado=true;
    }
  
}

void executarAcao() 
{
  switch (contaPalmas) {
    case 3: // Controlado pelas palmas
       digitalWrite(pinRele1, !digitalRead(pinRele1));
       Serial.println("Bateu Paumas");
       break;
    case 4: // Alterna entre controlado pelo som, ou pela temperatura
      somLigado=!somLigado;
      if(digitalRead(D4)==LOW){
        digitalWrite(D4,HIGH);  
      }else{
        digitalWrite(D4,LOW);  
      }
      Serial.println("Temperatura");
      break;   
  }
}
