#include <DHT11.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>


//declaraciones
const char* ssid     = "MiCasa";
const char* password = "1212cherokee";
const char* mqtt_server = "192.168.1.55";/*"IP Mosquitto server";*/

const int DHTPIN=14;
const int RED = 15;
const int GREEN = 12;
const int BLUE = 13;
int ldr=A0;
const int PINLLUVIA= 5;



/*Lista de topics*/
const char* TopicOut1 = "ExtTemperatura";
const char* TopicOut2 = "ExtHumedad";
const char* TopicOut3 = "ExtLux";
const char* TopicOut4 = "ExtClima";

DHT11 dht(DHTPIN);
WiFiClient espClient;
PubSubClient client(espClient);

/*Varaibles*/
String Clima="Seco";
int luminosidad;
boolean estado=false;
long lastMsg = 0;
char msg[50];
float h;
float t;
int timer = 0;
char charVal[10];               //temporarily holds data from vals 
String stringVal = "";     //data on buff is copied to this string
unsigned long previousMillisDHT = 0;        // will store last temp was read
const long intervalDHT = 2000; 

/* If a new message arrives, do this */
void callback(char* topic, byte* payload, unsigned int length) { /*Que hacer cuadno llega un mensaje*/
 //gestionar los mensajes recibidos en los topics subscritos    
   digitalWrite(BLUE,HIGH);
   Serial.print("Mensaje recibido [");  Serial.print(topic);  Serial.print("] ");
    String dato="";
    for (int i = 0; i < length ; i++) { 
        Serial.print((char)payload[i]);
        dato=dato+(char)payload[i];
    }
  Serial.println("");
   // Comprobar si hay que subir o bajar
    if ( dato.equals("Estado")) {
      estado=true;  
    } 
   digitalWrite(BLUE,LOW);    
 }

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    digitalWrite(GREEN,LOW);
    digitalWrite(RED,HIGH); 
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "Terraza";
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      digitalWrite(GREEN,HIGH);
      digitalWrite(RED,LOW);
      Serial.println("connected");
      // Once connected, resubscribe...
   
      client.subscribe(TopicOut1);
      client.subscribe(TopicOut2);
      client.subscribe(TopicOut3);
      client.subscribe(TopicOut4);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(ssid);
  //WiFi.config(ip,DNS,Gw,SubNet);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
void Publicar(){
    dtostrf(t, 4, 2, charVal);  
    client.publish(TopicOut1, charVal);
    dtostrf(h, 4, 2, charVal);  
    client.publish(TopicOut2, charVal);
    timer = 0;
    int ldrValue= analogRead(ldr);
    String datos = (String)ldrValue;
    datos.toCharArray(msg,10);
    client.publish(TopicOut3, msg);
    Clima.toCharArray(msg,10);
    client.publish(TopicOut4, msg);
  
}

    

void gettemperature() {
  // Wait at least 2 seconds seconds between measurements.
  // if the difference between the current time and last time you read
  // the sensor is bigger than the interval you set, read the sensor
  // Works better than delay for things happening elsewhere also
  unsigned long currentMillis = millis();
  if(currentMillis - previousMillisDHT >= intervalDHT) {
  int err;
  float t, h;
  if((err=dht.read(h, t))==0)
  {
    Serial.print("temperature:");
    Serial.print(t);
    Serial.print(" humidity:");
    Serial.print(h);
    Serial.println();
  }
  else
  {
    Serial.println();
    Serial.print("Error No :");
    Serial.print(err);
    Serial.println();    
  }
  
  }
}

void setup() {
    /* Add Event listeners */
    pinMode(RED, OUTPUT);
    pinMode(GREEN, OUTPUT);
    pinMode(BLUE, OUTPUT);
    pinMode(PINLLUVIA, INPUT);
    digitalWrite(BLUE,HIGH);
    Serial.begin(115200);
    Serial.println("Starting...");
    setup_wifi();
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);
    
    digitalWrite(BLUE,LOW);
   
}

void loop() {
 
    gettemperature();
    boolean lluvia = digitalRead(PINLLUVIA);
    if (lluvia){
     Clima="Mojado"; 
    } else {
     Clima="Seco";
    }
    if (!client.connected()) {
     reconnect();
    }
    client.loop();
    //enviar topics ciclicamente para refrescar datos
    long now = millis();  //temporizar publicaciones
    if (now - lastMsg > 10000) {
     lastMsg = now;
     Publicar();
    }
      
}





