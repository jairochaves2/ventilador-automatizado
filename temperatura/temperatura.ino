#include "DHT.h"
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

#define BLYNK_PRINT Serial

#define pinRele1 D0
#define pinSom D2
#define tempoMaximoDeUmaPalma  150 //milisegundos
#define tempoMaximoEntrePalmas 500 //milisegundos

//Token do Blynk
char auth[] = "bb395608a61c4ce591bb2ece7d2da5c0";

//Senhas Blink
char ssid[] = "SINAL FRACO";
char pass[] = "rosachaves1995";


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


long tempoEsperaEntrePalmas = 0;
bool somLigado =false;
void executarAcao();

DHT dht(D1, DHT11);
void setup() {
  Serial.begin(9600);
  dht.begin();
  Blynk.begin(auth, ssid, pass);
  
  pinMode(pinSom, INPUT);
  pinMode(pinRele1, OUTPUT);
  digitalWrite(pinRele1,HIGH);
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
  if((tempoFinal-tempoInicial)>20000 && somLigado==false){
    
    //Leitura de umidade
    float umidade = dht.readHumidity();
    //Leitura de temperatura
    float temperatura = dht.readTemperature();
    if(temperatura>34.00){
      digitalWrite(pinRele1,LOW);
    }if(isnan(temperatura)){
      Serial.println("Erro na Leitura do Sensor!");
    }else{
      digitalWrite(pinRele1,HIGH);
      
    }

    ///////////////////////////////////////////////////////////////////////
    //A cada 1 hora é feito um envio de notificação caso a humidade do ar esteja baixa
    tFinalBN=millis();
    if(umidade<40.00 && (tFinalBN-tInicialBN)>=(60000*60)){
        tInicialBN=millis();
        Blynk.notify("Atenção: Beba línquidos, Umidade do ar baixa");
      }
    ///////////////////////////////////////////////////////////////////////
      
    Serial.print("Temperatura: ");
    Serial.println(temperatura);
    Serial.print("Umidade: ");
    Serial.println(umidade);
    Serial.println();
    Serial.println(tempoFinal-tempoInicial);
    Serial.println(somLigado);
    ///////////////////////////////////////////////////////////////////////    
    tempoInicial=millis();
  }
  ///////////////////////////////////////////////////////////////////////  
  // Faz uma atualização no Blynk a cada 1 segundo e meio
  tFinalB=millis();
  if((tFinalB-tInicialB)>=1500){
    Blynk.run();
    tInicialB=millis();  
  }
  ///////////////////////////////////////////////////////////////////////  
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
      
      Serial.println("Temperatura");
      break;   
  }
}
