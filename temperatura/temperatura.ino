#include "DHT.h"

#define pinRele1 D0
#define pinSom D2
#define tempoMaximoDeUmaPalma  150 //milisegundos
#define tempoMaximoEntrePalmas 500 //milisegundos

int contaPalmas = 0;
long tempoEspera = 0;
long tempoInicial = 0;
long tempoFinal = 0;
long tempoEsperaEntrePalmas = 0;
bool somLigado =false;
void executarAcao();

DHT dht(D1, DHT11);
void setup() {
  Serial.begin(9600);
  dht.begin();
  
  pinMode(pinSom, INPUT);
  pinMode(pinRele1, OUTPUT);
  tempoInicial=millis();
}

void loop() {
//verifica o estado do sensor de som (ele fica normalmente com a porta ligada. Quando ouver uma palma, ele desliga momentaneamente a porta)
      
      int sensorSom = digitalRead(pinSom);
      
      //se o sensor detectou palmas
      if (sensorSom == LOW) {
    
         //espera um tempo para nao detectar a mesma palma mais de uma vez 
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
  tempoFinal=millis();
  if((tempoFinal-tempoInicial)>10000 && somLigado==false){
    
    //Leitura de umidade
    float umidade = dht.readHumidity();
    //Leitura de temperatura
    float temperatura = dht.readTemperature();
    if(temperatura>34.00){
      digitalWrite(pinRele1,LOW);
    }else{
      digitalWrite(pinRele1,HIGH);
      
    }
  
    Serial.print("Temperatura: ");
    Serial.println(temperatura);
    Serial.print("Umidade: ");
    Serial.println(umidade);
    Serial.println();
    Serial.println(tempoFinal-tempoInicial);
    Serial.println(somLigado);
    tempoInicial=millis();
  }
  
 // tempoFinal=millis();

  
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
