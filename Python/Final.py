import paho.mqtt.client as mqtt
import pyrebase

peso=0
tciclo=0
trobo=0
qualsabor=0
qualtamanho=0

firebaseConfig = {
  'apiKey': "AIzaSyA5jr_kD1pvJdvC1im_8GTFN05mgjlLu5s",
  'authDomain': "envasadora-tcc.firebaseapp.com",
  'databaseURL': "https://envasadora-tcc-default-rtdb.firebaseio.com",
  'projectId': "envasadora-tcc",
  'storageBucket': "envasadora-tcc.appspot.com",
  'messagingSenderId': "563955138299",
  'appId': "1:563955138299:web:764baf24950daec8027a5e",
  'measurementId': "G-Q31WBJZXE9"}
firebase = pyrebase.initialize_app(firebaseConfig)
db = firebase.database()

pmaiorlaranjabegin = db.child('QUANTIDADE').child('P-GRANDE-LARANJA').get()
pmaiorlimaobegin=db.child('QUANTIDADE').child('P-GRANDE-LIMAO').get()
pmenorlaranjabegin=db.child('QUANTIDADE').child('P-PEQUENO-LARANJA').get()
pmenorlimaobegin=db.child('QUANTIDADE').child('P-PEQUENO-LIMAO').get()
nprodutosbegin=db.child('QUANTIDADE').child('TOTAL').get()

pmaiorlaranja = pmaiorlaranjabegin.val()
pmaiorlimao = pmaiorlimaobegin.val()
pmenorlaranja = pmenorlaranjabegin.val()
pmenorlimao = pmenorlimaobegin.val()
nprodutos = nprodutosbegin.val()
print(pmaiorlaranja)
print(pmaiorlimao)
print(pmenorlaranja)
print(pmenorlimao)
print(nprodutos)

MQTT_BROKER = '192.168.43.242'
MQTT_PORT = 1883
KEEP_ALIVE_INTERVAL = 60

#SUBROTINA PARA CONECTAR
def on_connect(client, userdata, flag, rtc):
    mqttc.subscribe('roger', 1)
    mqttc.subscribe('TCC/ENVASADORA/ENVIAFIREBASE', 2)
    mqttc.subscribe('TCC/ENVASADORA/PRODUTO/TEMPOCICLO', 2)
    mqttc.subscribe('TCC/ENVASADORA/PRODUTO/TEMPOROBO', 2)
    mqttc.subscribe('TCC/ENVASADORA/PRODUTO/ABOUT', 2)
    mqttc.subscribe('TCC/ENVASADORA/PRODUTO/PESO', 2)
    mqttc.subscribe('TCC/ENVASADORA/PRODUTO/NPRODUTOS', 2)
    mqttc.subscribe('TCC/ENVASADORA/PRODUTO/MAIOR-LARANJA', 2)
    mqttc.subscribe('TCC/ENVASADORA/PRODUTO/MAIOR-LIMAO', 2)
    mqttc.subscribe('TCC/ENVASADORA/PRODUTO/MENOR-LARANJA', 2)
    mqttc.subscribe('TCC/ENVASADORA/PRODUTO/MENOR-LIMAO', 2)
    #print('Connected to MQTT')

#SUBROTINA PARA RECEBER
def on_message(client, userdata, msg):
    #print(msg.topic +' ' + str(msg.payload.decode('utf-8')))
    global nprodutos , pmaiorlaranja, pmaiorlimao, pmenorlaranja, pmenorlimao, peso, tciclo, trobo, qualsabor, qualtamanho
    value =int(msg.payload.decode('utf-8')) 
    print (msg.topic)
    print (value)

    if msg.topic == 'TCC/ENVASADORA/ENVIAFIREBASE' and value == 10:
        produto = 'PRODUTO '
        produto = produto+str(nprodutos)
        data = { 'P-GRANDE-LARANJA' : pmaiorlaranja , 'P-GRANDE-LIMAO':pmaiorlimao, 'P-PEQUENO-LARANJA': pmenorlaranja, 'P-PEQUENO-LIMAO': pmenorlimao, 'TOTAL':nprodutos }
        db.child("QUANTIDADE").set(data)

    if msg.topic == 'TCC/ENVASADORA/ENVIAFIREBASE' and value == 30:
        produto = 'PRODUTO '
        produto = produto+str(nprodutos)
        data1 = { 'VOLUME' : peso , 'TEMPO-CICLO':tciclo , 'TEMPO-ROBO': trobo, 'SABOR': qualsabor, 'TAMANHO':qualtamanho }
        db.child("PRODUTOS").child(produto).set(data1)

    if msg.topic == 'TCC/ENVASADORA/PRODUTO/PESO':
        peso = value
    
    if msg.topic == 'TCC/ENVASADORA/PRODUTO/NPRODUTOS':
        if value > nprodutos:
            nprodutos = value

    if msg.topic == 'TCC/ENVASADORA/PRODUTO/MAIOR-LARANJA':
        if value > pmaiorlaranja:
            pmaiorlaranja = value

    if msg.topic == 'TCC/ENVASADORA/PRODUTO/MAIOR-LIMAO':
        if value > pmaiorlimao:
            pmaiorlimao = value

    if msg.topic == 'TCC/ENVASADORA/PRODUTO/MENOR-LARANJA':
        if value > pmenorlaranja:
            pmenorlaranja = value

    if msg.topic == 'TCC/ENVASADORA/PRODUTO/MENOR-LIMAO':
        if value > pmenorlimao:
            pmenorlimao = value

    if msg.topic == 'TCC/ENVASADORA/PRODUTO/TEMPOCICLO':
        tciclo = (value/1000)
       
    if msg.topic == 'TCC/ENVASADORA/PRODUTO/TEMPOROBO':
        trobo = (value/1000)
        
    if msg.topic == 'TCC/ENVASADORA/PRODUTO/ABOUT':
        pabout = value
        if pabout == 10:
            qualsabor = 'LARANJA'
            qualtamanho = 'PEQUENO'
        
        if pabout == 20:
            qualsabor = 'LARANJA'
            qualtamanho = 'GRANDE'
        
        if pabout == 30:
            qualsabor = 'LIMAO'
            qualtamanho = 'PEQUENO'
        
        if pabout == 40:
            qualsabor = 'LIMAO'
            qualtamanho = 'GRANDE'

while True:
#EXECUÇÃO
    mqttc = mqtt.Client('PYTHON')

#CHAMADAS DE ROTINAS
    mqttc.on_message = on_message
    mqttc.on_connect = on_connect

    mqttc.connect(MQTT_BROKER,int(MQTT_PORT),int(KEEP_ALIVE_INTERVAL))
    mqttc.loop_forever()


