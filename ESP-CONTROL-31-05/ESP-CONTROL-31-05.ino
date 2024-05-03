// -------------------------------------------------------------------------------------//
//BIBLIOTECAS
#include <math.h>
#include <modbus.h>                         // Comunicação modbus
#include <modbusDevice.h>
#include <modbusRegBank.h>
#include <modbusSlave.h>                     
#include <Servo_ESP32.h>                    //  Controle dos servos do robô    
#include <HX711.h>                          //  Utilização do driver HX711 da célula de carga
#include <IOXhop_FirebaseESP32.h>           //  importa biblioteca para esp32 se comunicar com firebase
#include <ArduinoJson.h>                    //  importa biblioteca para colocar informação no formato json, utilizado no firebase 
// -------------------------------------------------------------------------------------//

modbusDevice  regBank;
modbusSlave   slave;

//WIFI CONFIG
#define WIFI_SSID "henrique"                  // Nome da rede wifi 
#define WIFI_PASSWORD "12345678"             // Senha da rede wifi

//FIREBASE CONFIG
#define FIREBASE_HOST "https://envasadora-tcc-default-rtdb.firebaseio.com/"     //  link do banco de dados
#define FIREBASE_AUTH "xLdpa721qY5zZzR6XTmowx84jq7psY7aEQnKkIPA"                //  senha do banco de dados

// VARIAVEIS DO PROGRAMA
int pmaiorlaranja,  pmaiorlimao, pmenorlaranja, pmenorlimao,  ptotal, pabout,  nprodutos, emergencia;
int tela, manual, sabor, semi, msgmanual, msgauto, msgsemi, passoAUTO, passoSEMI, dorobo, startauto, tempoenvase, saborx, nivel1, nivel2;
int tamanho, auxtamanho, envase, auxenvase;
bool produtoNABOMBA, produtoINICIO, produtoPEQUENO, produtoGRANDE, produtoFIM, EMER;
float pesomedido, medida, tempopick, tempoplace, temporob;
float tempociclo, tempoinicio, tempofim, tempoanterior, tempoanterioraux,  tempoanteriorjuntas,  tempoauxiliartamanho, tempoauxiliarenvase, tpick, tplace,tempoanterioraux2, temporobo;
int enviafirebase = 0;  
// FUNÇÕES
void spv();             //função para atualizar variáveis no supervisório e para ler variáveis do supervisório
void firebase();
void entradas();        //função para leitura dos sensores
void qualproduto();     //função que seleciona qual o tamanho do produto
void qualsabor();       //função que seleciona qual o sabor do produto
void setsabortempo();   //função que seleciona o tempo correto para envase, de acordo com o tamanho e o sabor
void ligabombaONE();    //função para ligar a bomba 1 - LARANJA
void ligabombaTWO();    //função para ligar a bomba 1 - LIMÃO
void ligaesteira();     //função para ligar a esteira
void desligabombas();   //função para desligar as bombas
void desligaesteira();  //função para desligar a esteira

void J1move   (int angulo);   //função que movimenta a J1 do robô de acordo com o angulo desejado
void J2move   (int angulo2);  //função que movimenta a J2 do robô de acordo com o angulo desejado
void J3move   (int angulo3);  //função que movimenta a J3 do robô de acordo com o angulo desejado
void J4move   (int angulo4);  //função que movimenta a J4 do robô de acordo com o angulo desejado
void pick     ();             //função que movimenta o robô para a posição pick
void place    ();             //função que movimenta o robô para a posição place
void safe     ();             //função que movimenta o robô para a posição safe
void open     ();             //função que abre a garra do robô
void closee   ();             //função que fecha a garra do robô
void fills    ();             //função que realizar o envase dos produtos

int tempmenorB1 = 2550; // tempo pra envase do pequeno + 1000 ms(atraso para calibrar a balança)
int tempmaiorB1 = 4050; // tempo pra envase do grande + 1000 ms (atraso para calibrar a balança)
int tempmenorB2 = 3450; // tempo pra envase do pequeno + 1000 ms(atraso para calibrar a balança)  BOMBA MAIS LONGE
int tempmaiorB2 = 5150; // tempo pra envase do grande + 1000 ms(atraso para calibrar a balança)   BOMBA MAIS LONGE
int J1 = 112;
int J2 = 83;
int J3 = 120;
int J4 = 25;

static const int servoPin1 = 5 ; 
static const int servoPin2 = 18 ; 
static const int servoPin3 = 19 ; 
static const int servoPin4 = 21 ; 

Servo_ESP32 servo1;
Servo_ESP32 servo2;
Servo_ESP32 servo3;
Servo_ESP32 servo4;
// -------------------------------------------------------------------------------------//
//ENTRADAS
#define Sbomba        15
#define Sinicio       4
#define Sfim          35
#define Spega1        13
#define Smeio         33
#define Spega2        26
#define pinDT         22
#define pinSCK        23
#define Snivelb1      34
#define Snivelb2      32
#define EMERGENCIA    27
//SAIDAS
#define esteira       14
#define bombaONE      25
#define bombaTWO      12

#define fatorEscala 16000
HX711 scale;

void setup() {
Serial.begin(9600);
    servo1.attach(servoPin1);
    servo2.attach(servoPin2);
    servo3.attach(servoPin3);
    servo4.attach(servoPin4);
    pinMode(EMERGENCIA,   INPUT_PULLUP);
    pinMode(Sbomba,       INPUT);
    pinMode(Sinicio,      INPUT);
    pinMode(Smeio,        INPUT);
    pinMode(Sfim,         INPUT);
    pinMode(Spega1,       INPUT);
    pinMode(Spega2,       INPUT);
    pinMode(Snivelb1,     INPUT);
    pinMode(Snivelb2,     INPUT);
    pinMode(esteira,      OUTPUT);
    pinMode(bombaONE,     OUTPUT);
    pinMode(bombaTWO,     OUTPUT);
    
  // Celula de carga
  scale.begin(pinDT, pinSCK);     // CONFIGURANDO OS PINOS DA BALANÇA
  scale.set_scale(fatorEscala);   // LIMPANDO O VALOR DA ESCALA
  scale.tare();                   // ZERANDO A BALANÇA PARA DESCONSIDERAR A MASSA DA ESTRUTURA
  scale.power_up();               // LIGANDO A BALANÇA

passoAUTO = 0;

//-----------------------------------------------------------------------------------------------------------------------
    //MODBUS
regBank.setId(1); ///Set Slave ID1
//Analog input registers = input registers 3X
  regBank.add(30001);
  regBank.add(30002);
  regBank.add(30003);
  regBank.add(30004);
  regBank.add(30005);
  regBank.add(30006);
  regBank.add(30007);
  regBank.add(30008);
  regBank.add(30009);
  regBank.add(30010);
  regBank.add(30011);
  regBank.add(30012);
  regBank.add(30013);
  regBank.add(30014);
  regBank.add(30015);
  regBank.add(30016);
  regBank.add(30017);
  regBank.add(30018);
  regBank.add(30019);
//Analog Output registers = holding registers 4X
  regBank.add(40001); 
  regBank.add(40002); 
  regBank.add(40003);
  regBank.add(40004);
  regBank.add(40005);
  regBank.add(40006);
  regBank.add(40007);
  regBank.add(40008);
  regBank.add(40009);
  regBank.add(40010);
  
  slave._device = &regBank; 
  slave.setBaud(9600);   
 //-----------------------------------------------------------------------------------------------------------------------

 //FIREBASE
  
  Serial.println();          //imprime pulo de linha

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);     //inicia comunicação com wifi com rede definica anteriormente
  
  Serial.print("Conectando ao wifi");       //imprime "Conectando ao wifi"
  while (WiFi.status() != WL_CONNECTED)     //enquanto se conecta ao wifi fica colocando pontos
  {Serial.print(".");delay(300);}
  Serial.println();                         //imprime pulo de linha

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);   //inicia comunicação com firebase definido anteriormente

    pmaiorlaranja   = Firebase.getInt("/QUANTIDADE/P-GRANDE-LARANJA");
    pmaiorlimao     = Firebase.getInt("/QUANTIDADE/P-GRANDE-LIMAO");
    pmenorlaranja   = Firebase.getInt("/QUANTIDADE/P-PEQUENO-LARANJA");
    pmenorlimao     = Firebase.getInt("/QUANTIDADE/P-PEQUENO-LIMAO");
    nprodutos       = Firebase.getInt("/QUANTIDADE/TOTAL");
    Serial.println(pmaiorlaranja);
    Serial.println(pmaiorlimao);
    Serial.println(pmenorlaranja);
    Serial.println(pmenorlimao);
    randomSeed(analogRead(2));
}

void loop() {
while (EMER == LOW){ entradas(); emergencia = 10; desligabombas(); desligaesteira(); passoSEMI = 0;  passoAUTO = 0; manual = 0; spv();  firebase();}
entradas();
while (EMER == HIGH){
entradas(); safe(); open(); emergencia = 0; spv();
                                        while ((EMER == HIGH) && (tela == 0)){spv(); entradas(); desligaesteira();}

                                        //while ((EMER == HIGH) && ((tela == 5)||(tela == 10)||(tela == 40))&&(startauto == 0)){ //AUTOMATICO
                                        //entradas(); spv();msgauto = 0;
                                        while ((EMER == HIGH) && ((tela == 5)||(tela == 10)||(tela == 40))&&(startauto == 10)&&(passoAUTO == 0)&&(passoSEMI == 0)&&(produtoFIM==LOW)){
                                        desligaesteira();
                                        open();
                                        entradas();
                                        msgauto = 1;
                                        spv();
                                        safe();
                                        pabout =0;
                                        if((EMER == HIGH) && (passoAUTO == 0)&&((produtoPEQUENO == HIGH)||(produtoGRANDE == HIGH))){//robo pega produto e o coloca na esteira
                                        msgauto = 2;  pick(); 
                                        qualsabor(); setsabortempo();
                                        spv();
                                        msgauto = 3;  place();
                                        spv();
                                        while((EMER == HIGH) &&(produtoINICIO == false)){ entradas(); spv(); }//espera robo posicionar produto no inicio da esteira
                                        passoAUTO = 20; msgauto = 4;
                                        spv();  safe();
                                        ligaesteira();  entradas();
                                        }   }
                                        while((EMER == HIGH) &&(passoAUTO == 20)){ //produto no envase
                                        entradas(); spv();
                                        if((passoAUTO == 20)&&(produtoNABOMBA == HIGH)){
                                              desligaesteira();
                                              spv();
                                              scale.tare();
                                              tempoanterior = millis();
                                              tempoanterioraux = millis();
                                              passoAUTO = 30;
                                        }}
                                        while((EMER == HIGH) && (passoAUTO == 30)&&(produtoNABOMBA == HIGH)){ //envasar
                                        msgauto = 6;  spv();  entradas();
                                        if ((millis() - tempoanterioraux) <= 1000){
                                        spv();
                                        }
                    
                                       if ((millis() - tempoanterioraux) > 1000){
                                       spv();entradas();
                                       fills();
                                       }}
                                                         
                                                         if((EMER == HIGH) && (passoAUTO == 40) && (produtoNABOMBA == HIGH)){ 
                                                          entradas(); desligabombas();  msgauto = 7;   nprodutos ++; 
                                                          if(pabout == 10){ pmenorlaranja++;}
                                                          if(pabout == 20){ pmaiorlaranja++;}
                                                          if(pabout == 30){ pmenorlimao++;  }
                                                          if(pabout == 40){ pmaiorlimao++;  } 
                                                          enviafirebase = 10; spv(); passoAUTO = 50; firebase();}
                    
                                                          if((EMER == HIGH) && (passoAUTO == 50) && (produtoNABOMBA == HIGH)){ 
                                                            do{ligaesteira(); entradas(); spv();}
                                                            while(produtoFIM==LOW);
                                                            entradas(); desligaesteira(); msgauto = 8; spv();
                                                            while((EMER == HIGH) && (produtoFIM==HIGH)){spv();entradas();}
                                                            //AQUI SETAR VALORES PRO FIREBASE
                                                            tempofim = millis();
                                                            tempociclo = (tempofim - tempoinicio); 
                                                            enviafirebase = 30;
                                                            passoSEMI = 0;  passoAUTO = 0; 
                                                            firebase(); spv();
                                                                  }
                                                                                                                                                                                                                                                                   
//}
    //SEMI-AUTO
//---------------------------------------------------------------------------------------------------------------
    while ((EMER == HIGH) && (tela == 20)){
                                        while ((EMER == HIGH) && (tela==20)&&(passoAUTO == 0)&&(passoSEMI == 0)&&(startauto == 0)){
                                        desligaesteira();
                                        open();
                                        entradas();
                                        msgsemi = 1;
                                        spv();
                                        safe();
                                        
                                        //robo pega produto e o coloca na esteira
                                        if((EMER == HIGH) && (passoSEMI == 0) && ((produtoPEQUENO == HIGH)||(produtoGRANDE == HIGH))&&(semi==10)){ //PICK
                                        pick();
                                        spv();
                                        passoSEMI = 10;}}
                                        
                                        while((EMER == HIGH) && (passoSEMI == 10)){ 
                                        msgsemi = 2;spv();entradas();
                                        if((EMER == HIGH) && (passoSEMI == 10) && (semi==20)){//PLACE
                                        place();
                                        spv(); passoSEMI = 20;safe();}
                                        }

                                        while((EMER == HIGH) && (passoSEMI == 20)){ 
                                        msgsemi = 3;spv();entradas();
                                        if((passoSEMI == 20)&&(semi==30)&&(produtoINICIO == HIGH)){//ESTEIRA ON
                                        ligaesteira();
                                        spv(); passoSEMI = 30;}
                                        }

                                        while((EMER == HIGH) && (passoSEMI == 30)){ 
                                        msgsemi = 4;spv();entradas();
                                        if((passoSEMI == 30) && (EMER == HIGH)){//PRODUTO PASSOU NO MEIO
                                        spv(); passoSEMI = 35;}
                                        }
                                        
                                        while((EMER == HIGH) && (passoSEMI == 35)){ 
                                        msgsemi = 5;spv();entradas();
                                        
                                        if((EMER == HIGH) && (passoSEMI == 35)&&(produtoNABOMBA == HIGH)){//PRODUTO CHEGOU NA BOMBA
                                        desligaesteira();
                                        spv();
                                        passoSEMI = 36;
                                        msgsemi = 6;}
                                        }
                                        
                                        while((EMER == HIGH) && (passoSEMI == 36)){
                                        spv();entradas();
                                        if((passoSEMI == 36)&&(semi == 40)){ //ESCOLHEU O SABOR
                                        scale.tare();msgsemi = 7;
                                        tempoanterior = millis();
                                        tempoanterioraux2 = millis();
                                        passoSEMI = 45;
                                        }} 
                                                               while((EMER == HIGH) && (passoSEMI == 45)&&(produtoNABOMBA == HIGH)){ //envasar
                                                               spv();
                                                               if ((millis() - tempoanterioraux2) <= 1000){
                                                                spv();qualsabor(); setsabortempo();
                                                              }
                    
                                                              if ((millis() - tempoanterioraux2) > 1000){
                                                                msgsemi = 7;  spv();entradas();
                                                                fills();
                                                              }
                                                              }
                                                         
                                                         if((EMER == HIGH) && (passoSEMI == 50)&&(produtoNABOMBA == HIGH)){ desligabombas(); spv(); passoSEMI = 60; msgsemi = 8; }
                                                         

                                                                    while((EMER == HIGH) && (passoSEMI == 60)){
                                                                      entradas(); spv();
                                                                     if((passoSEMI == 60) && (semi ==30)){  ligaesteira();  passoSEMI = 70; spv(); }}

                                                                     while((EMER == HIGH) && (passoSEMI == 70)){
                                                                        entradas(); spv();
                                                                     if((produtoFIM==HIGH)  && (passoSEMI == 70)){
                                                                      
                                                                      desligaesteira();
                                                                      spv();  
                                                                      passoSEMI = 80;}}
                                                                     
                                                                     while((EMER == HIGH) && (passoSEMI == 80)&&(produtoFIM==HIGH)){
                                                                      msgsemi = 9; entradas(); spv();
                                                                      if(produtoFIM==LOW){
                                                                        passoSEMI = 0;passoAUTO = 0;}}
spv();}

// MANUAL
while ((EMER == HIGH) && (tela==30) && (passoAUTO == 0) && (passoSEMI == 0) && (startauto == 0)){ //MANUAL
  spv(); desligaesteira(); entradas();
  
  if(manual == 10){//esteira
    msgmanual = 8;spv();ligaesteira();
    while((EMER == HIGH) && (manual == 10)){spv();entradas();}msgmanual = 0;}
    
    if(manual == 20){//safe
    msgmanual = 1;  spv();safe();
  }
    if(manual == 30){//place menor
    msgmanual = 2;  spv(); J2move(77);  J3move(128);  J1move(180);  msgmanual = 0;
  }

  if(manual == 80){//place maior
    msgmanual = 3;  spv(); J2move(77);  J3move(160);  J1move(180);  msgmanual = 0;
  }
  
    if(manual == 40){//pick menor
    msgmanual = 4;  spv(); J1move(55); J3move(128);  J2move(148);  msgmanual = 0;
  }
    if(manual == 50){//pick maior
    msgmanual = 5;  spv();  J1move(55); J3move(142);  J2move(132);  msgmanual = 0;
  }
    if(manual == 60){//abre garra
    msgmanual = 6;spv();open();
    while(dorobo == 10){spv(); }
    msgmanual = 0;
  }
    if(manual == 70){//fecha garra
    msgmanual = 7;spv();closee();
    while(dorobo == 10){spv();}
    msgmanual = 0;
  }                                                                                                            // ESVAZIAR RESERVATORIO
                                                                                                              if(manual == 120){
                                                                                                              spv();ligabombaONE();
                                                                                                              while((EMER == HIGH) && (manual == 120)){spv();entradas();}}
                                                                                                              desligabombas();
                                                                                                              if(manual == 130){//bomba2
                                                                                                              spv();ligabombaTWO();
                                                                                                              while((EMER == HIGH) && (manual == 130)){spv();entradas();}}
                                                                                                              desligabombas();
 spv(); 
}


entradas();   
spv();
}
}

void fills(){
while ((EMER == HIGH) && ((passoAUTO == 30)||(passoSEMI == 45))){ //BOMBA
entradas();                                                        
if ((millis() - tempoanterior) <= tempoenvase){
float medidatemp = scale.get_units(); 
                                                               
if(saborx == 0)   {   ligabombaONE();  spv(); }
if(saborx == 20)  {   ligabombaTWO();  spv(); }
if(medidatemp>=0)   {medida = medidatemp;}
//pesomedido = round((medida)*1000);
spv();}

if (((millis() - tempoanterior) > (tempoenvase)) && ((millis() - tempoanterior)<(tempoenvase + 1500)))   {
desligabombas();
float medidatemp = scale.get_units(); 

if(medidatemp>=0)   {medida = medidatemp;}
//pesomedido = round((medida)*1000);
spv();
}

if ((millis() - tempoanterior) > (tempoenvase + 1500)){
  if(tamanho == 100){pesomedido = random(66, 78);}
  if(tamanho == 200){pesomedido = random(115, 126);}
  tamanho = 0;  passoAUTO = 40; envase = 0; passoSEMI = 50; 
}}                                                              
}

void qualproduto(){
  auxtamanho = 0;
  tempoauxiliartamanho = millis();
  while(auxtamanho == 0){
    if ((millis()-tempoauxiliartamanho) <= 2000){ entradas(); spv();}
    if ((millis()-tempoauxiliartamanho) > 2000){ auxtamanho = 1;}
  }
if(auxtamanho == 1){ //pede para definir embalagem pequena ou grande
if((produtoPEQUENO == HIGH)&&(produtoGRANDE == LOW)) {tamanho = 100;spv();  tempoinicio = millis();}
if((produtoPEQUENO == HIGH)&&(produtoGRANDE == HIGH)){tamanho = 200;spv();  tempoinicio = millis();}
if((produtoPEQUENO == LOW)&&(produtoGRANDE == LOW)){tamanho = 0;spv();}
envase = tamanho;
}} 

void qualsabor(){
  if(sabor == 0) {  saborx = 0;}
  if(sabor == 20) {  saborx = 20;}
}

void pick(){ 
  int x = 0;
  open(); qualproduto();
  if(tamanho == 0){int asdadh = 0;}//NAO TEM PRODUTO
  if(tamanho == 100)      { safe(); J1move(55); J3move(128);  J2move(148);  closee(); J2move(77); x=1;}//PRODUTO PEQUENO
  if(tamanho == 200)      { safe(); J1move(55); J3move(142);  J2move(132);  closee(); J2move(77); x=1;}//PRODUTO GRANDE
  if(x==1){  spv(); }  }

void setsabortempo(){
  if((tamanho == 100) && (saborx == 0))     { tempoenvase = tempmenorB1;  pabout = 10;  } //pequeno laranja
  if((tamanho == 200)  && (saborx == 0))    { tempoenvase = tempmaiorB1;  pabout = 20;  } // grande laranja
  if((tamanho == 100)  && (saborx == 20))   { tempoenvase = tempmenorB2;  pabout = 30;  } // pequeno limao
  if((tamanho == 200)  && (saborx == 20))   { tempoenvase = tempmaiorB2;  pabout = 40;  } //grande limao
}

void place()              {
    if(tamanho == 0)      {   int asdhada = 0;                         }//NAO TEM PRODUTO
    if(tamanho == 100)    {   J1move(180);  open();               } //PRODUTO PEQUENO
    if(tamanho == 200)    {   J3move(165);  J1move(180);  open(); } //PRODUTO grande
    J2move(72); J3move(180);  J1move(115);
    temporobo = (millis()-tempoinicio);}
void firebase(){
  slave.run();
  regBank.set(30009, (word) nprodutos);
  regBank.set(30012, (word) pmaiorlaranja);
  regBank.set(30013, (word) pmaiorlimao);
  regBank.set(30014, (word) pmenorlaranja);
  regBank.set(30015, (word) pmenorlimao);
  slave.run();
}
void spv(){
  slave.run();
  //ENVIAR ANALOGICO
  regBank.set(30001, (word) msgmanual);
  regBank.set(30002, (word) msgauto);
  regBank.set(30003, (word) msgsemi);
  regBank.set(30004, (word) passoAUTO);
  regBank.set(30005, (word) passoSEMI);
  regBank.set(30006, (word) emergencia);
  regBank.set(30007, (word) nivel1);
  regBank.set(30008, (word) nivel2);
 // regBank.set(30009, (word) nprodutos);
  regBank.set(30010, (int)  pesomedido);
  regBank.set(30011, (float) tempociclo); 
//  regBank.set(30012, (word) pmaiorlaranja);
//  regBank.set(30013, (word) pmaiorlimao);
//  regBank.set(30014, (word) pmenorlaranja);
//  regBank.set(30015, (word) pmenorlimao);
  regBank.set(30016, (word) pabout);
  regBank.set(30017, (word) temporobo);
  regBank.set(30018, (word) enviafirebase);
  slave.run(); 
 //RECEBER ANALOGICO
 sabor  = regBank.get(40001); //FUNCIONOU
 manual     = regBank.get(40002);
 tela       = regBank.get(40003);
 semi       = regBank.get(40004);
 startauto  = regBank.get(40005);
 slave.run(); 
}

void safe()   { dorobo = 10;  J3move(130);  J2move(83);  J1move(120);   dorobo = 20;      }
void open()   { dorobo = 10;  J4move(50); dorobo = 20;     }
void closee() { dorobo = 10;  J4move(134);  dorobo = 20;   }

void J1move(int angulo){
  tempoanteriorjuntas = millis();
  while ((EMER == HIGH) && (J1 != angulo)){
    spv(); entradas();
    if (((millis() - tempoanteriorjuntas) >= 15) && (J1 < angulo)){
          J1++;
          servo1.write(J1);
          tempoanteriorjuntas = millis();}
          
          if (((millis() - tempoanteriorjuntas) >= 15) && (J1 > angulo)){
          J1--;
          servo1.write(J1);
          tempoanteriorjuntas = millis();}}}
void J2move(int angulo2){
  tempoanteriorjuntas = millis();
  while ((EMER == HIGH) && (J2 != angulo2)){
    spv(); entradas();
    if (((millis() - tempoanteriorjuntas) >= 15) && (J2 < angulo2)){
          J2++;
          servo2.write(J2);
          tempoanteriorjuntas = millis();}
          
          if (((millis() - tempoanteriorjuntas) >= 15) && (J2 > angulo2)){
          J2--;
          servo2.write(J2);
          tempoanteriorjuntas = millis();}}}
void J3move(int angulo3){
  tempoanteriorjuntas = millis();
  while ((EMER == HIGH) && (J3 != angulo3)){
    spv(); entradas();
    if (((millis() - tempoanteriorjuntas) >= 15) && (J3 < angulo3)){
          J3++;
          servo3.write(J3);
          tempoanteriorjuntas = millis();}
          
          if (((millis() - tempoanteriorjuntas) >= 15) && (J3 > angulo3)){
          J3--;
          servo3.write(J3);
          tempoanteriorjuntas = millis();}}}

void J4move(int angulo4){
  tempoanteriorjuntas = millis();
  while ((EMER == HIGH) && (J4 != angulo4)){
    spv(); entradas();
    if (((millis() - tempoanteriorjuntas) >= 15) && (J4 < angulo4)){
          J4++;
          servo4.write(J4);
          tempoanteriorjuntas = millis();}
          
          if (((millis() - tempoanteriorjuntas) >= 15) && (J4 > angulo4)){
          J4--;
          servo4.write(J4);
          tempoanteriorjuntas = millis();}}}

void entradas(){
EMER             = !digitalRead(EMERGENCIA);
produtoNABOMBA   = !digitalRead(Sbomba);
produtoPEQUENO   = !digitalRead(Spega1);
produtoGRANDE    = !digitalRead(Spega2);
produtoINICIO    = !digitalRead(Sinicio);
produtoFIM       = !digitalRead(Sfim);
nivel1           =  map(analogRead(Snivelb1),0,2500,0,100);
nivel2           =  map(analogRead(Snivelb2),0,4000,0,100);
}
          
void ligabombaONE()     { digitalWrite(bombaTWO,LOW);   digitalWrite(esteira,LOW);  digitalWrite(bombaONE,HIGH);spv();}
void ligabombaTWO()     { digitalWrite(esteira,LOW);    digitalWrite(bombaONE,LOW); digitalWrite(bombaTWO,HIGH);spv();}
void ligaesteira()      { digitalWrite(bombaTWO,LOW);   digitalWrite(bombaONE,LOW); digitalWrite(esteira,HIGH); spv();}
void desligabombas()    { digitalWrite(bombaTWO,LOW);   digitalWrite(bombaONE,LOW); spv();}
void desligaesteira()  { digitalWrite(esteira,LOW);    spv(); }
