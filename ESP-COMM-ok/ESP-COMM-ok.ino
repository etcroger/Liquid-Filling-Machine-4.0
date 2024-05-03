
#include <WiFi.h>                          //importa biblioteca para conectar esp32 com wifi
#include <IOXhop_FirebaseESP32.h>          //importa biblioteca para esp32 se comunicar com firebase
#include <ArduinoJson.h>                   //importa biblioteca para colocar informação no formato json, utilizado no firebase (intalar versão 5)
#include <PubSubClient.h>

const char* SSID = "henrique";               // SSID / nome da rede WiFi que deseja se conectar      "Moto"  ;
const char* PASSWORD = "12345678" ;         // Senha da rede WiFi que deseja se conectar            "12345678"; 

#define FIREBASE_HOST "https://envasadora-tcc-default-rtdb.firebaseio.com/"    //substitua "Link_do_seu_banco_de_dados" pelo link do seu banco de dados 
#define FIREBASE_AUTH "xLdpa721qY5zZzR6XTmowx84jq7psY7aEQnKkIPA"   //substitua "Senha_do_seu_banco_de_dados" pela senha do seu banco de dados

WiFiClient wifiClient;                        
 
//MQTT Server
//const char* BROKER_MQTT = "192.168.0.7";//test.mosquitto.org"; //URL do broker MQTT que se deseja utilizar
const char* BROKER_MQTT = "192.168.0.6"; //URL do broker MQTT que se deseja utilizar
int BROKER_PORT = 1883;                      // Porta do Broker MQTT
#define ID_MQTT  "ESP-COM"             //Informe um ID unico e seu. Caso sejam usados IDs repetidos a ultima conexão irá sobrepor a anterior. 

PubSubClient MQTT(wifiClient);        // Instancia o Cliente MQTT passando o objeto espClient

//Declaração das Funções
void mantemConexoes();  //Garante que as conexoes com WiFi e MQTT Broker se mantenham ativas
void conectaWiFi();     //Faz conexão com WiFi
void conectaMQTT();     //Faz conexão com Broker MQTT
void recebePacote(char* topic, byte* payload, unsigned int length);

String qualsabor, qualtamanho;
String produto = "PRODUTO ";
int recebe,evitarepetirsprodutos, evitarepetirqtde, qtdok,pabout,peso,tciclo,trobo;
int nprodutos, enviafirebase, evitarepetirsensores, pmaiorlaranja,pmaiorlimao, pmenorlaranja, pmenorlimao, nivel1, nivel2;
float tempoanterioraux;
float teste, tciclos, trobos;
void setup() {
  Serial.begin(115200);      //inicia comunicação serial

  conectaWiFi();
  MQTT.setServer(BROKER_MQTT, BROKER_PORT);  
  MQTT.setCallback(recebePacote); 
  }

void loop() {
  mantemConexoes();
  MQTT.loop();

if((recebe == 10)&&(evitarepetirqtde == 0)){
  Firebase.setInt("/QUANTIDADE/P-GRANDE-LIMAO",    pmaiorlimao);
  Firebase.setInt("/QUANTIDADE/P-PEQUENO-LARANJA",  pmenorlaranja);
  Firebase.setInt("/QUANTIDADE/P-PEQUENO-LIMAO",    pmenorlimao);
  Firebase.setInt("/QUANTIDADE/P-GRANDE-LARANJA",  pmaiorlaranja);
  Firebase.setInt("/QUANTIDADE/TOTAL",            nprodutos);
  evitarepetirqtde = 10; qtdok = 0;
}
  
if((recebe == 30)&&(evitarepetirsprodutos == 0)){
produto += nprodutos;
Firebase.setInt("/PRODUTOS/"+produto+"/VOLUME",        peso);
Firebase.setFloat("/PRODUTOS/"+produto+"/TEMPO-CICLO", tciclos);
Firebase.setFloat("/PRODUTOS/"+produto+"/TEMPO-ROBO",  trobos);
Firebase.setString("/PRODUTOS/"+produto+"/SABOR",    qualsabor);
Firebase.setString("/PRODUTOS/"+produto+"/TAMANHO",  qualtamanho);
evitarepetirsprodutos = 10;  produto = "PRODUTO ";
}


MQTT.loop();
}

void recebePacote(char* topic, byte* payload, unsigned int length) 
{
    String msg;

    //obtem a string do payload recebido
    for(int i = 0; i < length; i++) 
    {
       char c = (char)payload[i];
       msg += c;
    }
//    if (String(topic) == "TCC/ENVASADORA/NIVEL1"){  nivel1 = msg.toInt(); }
//
//    if (String(topic) == "TCC/ENVASADORA/NIVEL2"){  nivel2 = msg.toInt(); }
    
    if (String(topic) == "TCC/ENVASADORA/ENVIAFIREBASE"){
      recebe = msg.toInt();
      Serial.println("");
      Serial.println(topic);
      Serial.println(recebe);
      //quantidades
      if (recebe == 10){ evitarepetirqtde = 0;  }
      
      //produtos
      if (recebe == 30){ evitarepetirsprodutos = 0;}
    }

      if (String(topic) == "TCC/ENVASADORA/PRODUTO/PESO"){
      peso = msg.toInt();  
    }

    if (String(topic) == "TCC/ENVASADORA/PRODUTO/NPRODUTOS"){
      if(msg.toInt() > nprodutos){    nprodutos = msg.toInt();  }
      Serial.println("");
      Serial.println(topic);
      Serial.println(nprodutos);
    }

      if (String(topic) == "TCC/ENVASADORA/PRODUTO/MAIOR-LARANJA"){
        if(msg.toInt() > pmaiorlaranja){    pmaiorlaranja = msg.toInt();  }   
    }

      if (String(topic) == "TCC/ENVASADORA/PRODUTO/MAIOR-LIMAO"){
        if(msg.toInt() > pmaiorlimao){    pmaiorlimao = msg.toInt();  }
    }

      if (String(topic) == "TCC/ENVASADORA/PRODUTO/MENOR-LARANJA"){
        if(msg.toInt() > pmenorlaranja){    pmenorlaranja = msg.toInt();  }
    }

      if (String(topic) == "TCC/ENVASADORA/PRODUTO/MENOR-LIMAO"){
        if(msg.toInt() > pmenorlimao){    pmenorlimao = msg.toInt();  }
    }
    if (String(topic) == "TCC/ENVASADORA/PRODUTO/ABOUT"){
      pabout = msg.toInt();
      Serial.println("");
      Serial.println(topic);
      Serial.println(pabout);
      
    if (pabout == 10){  qualsabor = "LARANJA";  qualtamanho = "PEQUENO";  }
    if (pabout == 20){  qualsabor = "LARANJA";  qualtamanho = "GRANDE";  }
    if (pabout == 30){  qualsabor = "LIMAO";  qualtamanho = "PEQUENO";  }
    if (pabout == 40){  qualsabor = "LIMAO";  qualtamanho = "GRANDE";  }
    }
    if (String(topic) == "TCC/ENVASADORA/PRODUTO/TEMPOCICLO"){
      tciclo = msg.toInt();
      tciclos = ((tciclo/10000.0)*10.0);
      Serial.println("");
      Serial.println(topic);
      Serial.println(tciclos);
    }
    if (String(topic) == "TCC/ENVASADORA/PRODUTO/TEMPOROBO"){
      trobo = msg.toInt();
      trobos = ((trobo/10000.0)*10.0);
      Serial.println("");
      Serial.println(topic);
      Serial.println(trobos);
    }
    
}

void mantemConexoes() {
    if (!MQTT.connected()) {
       conectaMQTT(); 
    }
    
    conectaWiFi(); //se não há conexão com o WiFI, a conexão é refeita
}

void conectaWiFi() {

  if (WiFi.status() == WL_CONNECTED) {
     return;
  }
        
  Serial.print("Conectando-se na rede: ");
  Serial.print(SSID);
  Serial.println("  Aguarde!");

  WiFi.begin(SSID, PASSWORD); // Conecta na rede WI-FI  
  while (WiFi.status() != WL_CONNECTED) {
      delay(100);
      Serial.print(".");
  }
  
  Serial.println();
  Serial.print("Conectado com sucesso, na rede: ");
  Serial.print(SSID);  
  Serial.print("  IP obtido: ");
  Serial.println(WiFi.localIP()); 
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
} 

void conectaMQTT() { 
    while (!MQTT.connected()) {
        Serial.print("Conectando ao Broker MQTT: ");
        Serial.println(BROKER_MQTT);
        if (MQTT.connect(ID_MQTT)) {
            Serial.println("Conectado ao Broker com sucesso!");
            MQTT.subscribe("TCC/ENVASADORA/ENVIAFIREBASE");
//            MQTT.subscribe("TCC/ENVASADORA/NIVEL1");
//            MQTT.subscribe("TCC/ENVASADORA/NIVEL2");
            
            MQTT.subscribe("TCC/ENVASADORA/PRODUTO/TEMPOCICLO");
            MQTT.subscribe("TCC/ENVASADORA/PRODUTO/TEMPOROBO");
            MQTT.subscribe("TCC/ENVASADORA/PRODUTO/ABOUT");
            MQTT.subscribe("TCC/ENVASADORA/PRODUTO/PESO");
            MQTT.subscribe("TCC/ENVASADORA/PRODUTO/NPRODUTOS");
            MQTT.subscribe("TCC/ENVASADORA/PRODUTO/MAIOR-LARANJA");
            MQTT.subscribe("TCC/ENVASADORA/PRODUTO/MAIOR-LIMAO");
            MQTT.subscribe("TCC/ENVASADORA/PRODUTO/MENOR-LARANJA");
            MQTT.subscribe("TCC/ENVASADORA/PRODUTO/MENOR-LIMAO");
            
tempoanterioraux = millis();
        } 
        else {
            Serial.println("Nao foi possivel se conectar ao broker.");
            Serial.println("Nova tentatica de conexao em 5s");
            delay(5000);
        }
    }
}
