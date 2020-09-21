#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid     = "Familia Ochoa";
const char* password = "0301072468";

const char *mqtt_server = "techsolutionsec.ga";
const int mqtt_port = 1883;
const char *mqtt_user = "web_client";
const char *mqtt_pass = "140597";

WiFiClient espClient;
PubSubClient client(espClient);

long lastMsg = 0;
char msg[40];

float tempESP32 = 0;
String estadoLED;
String estadoTouch;

bool estado_led_last=true;
bool estado_led_act=false;
bool estadoTouch_last=true;
bool estadoTouch_act=false;



//*****************************
//*** Sensor Temperatura ******
//*****************************

#ifdef __cplusplus
extern "C" {
#endif
uint8_t temprature_sens_read();
#ifdef __cplusplus
}
#endif
uint8_t temprature_sens_read();


//*****************************
//*** DECLARACION FUNCIONES ***
//*****************************
void setup_wifi();
void callback(char* topic, byte* payload, unsigned int length);
void reconnect();

void setup() 
{
	pinMode(BUILTIN_LED, OUTPUT);
	Serial.begin(9600);
	randomSeed(micros());
	setup_wifi();
	client.setServer(mqtt_server, mqtt_port);
	client.setCallback(callback);
}

void loop() {
	if (!client.connected()) 
  {
		reconnect();
	}

	client.loop();

  // *******************************
  // ******** PUBLICACIONES ********
  // *******************************
  
  if(digitalRead(BUILTIN_LED))
  {
    estado_led_act = true;
  }
  else
  {
    estado_led_act = false;
  }

  if(estado_led_last != estado_led_act)
  {
    String to_send_led;
    estado_led_last = estado_led_act;
    if(estado_led_act)
    {
      to_send_led = "ENCENDIDO";
    } 
    else
    {
      to_send_led = "APAGADO";
    }
    to_send_led.toCharArray(msg,10);
    client.publish("status_led", msg);
  }


  if(touchRead(15)<20)
  {
    estadoTouch_act = true;
  }
  else
  {
    estadoTouch_act = false;
  }

  if(estadoTouch_last != estadoTouch_act)
  {
    delay(250);
    String to_sendTouch;
    estadoTouch_last = estadoTouch_act;
    if(estadoTouch_act)
    {
      to_sendTouch = "ACTIVADO";
    } 
    else
    {
      to_sendTouch = "DESACTIVADO";
    }
    to_sendTouch.toCharArray(msg,12);
    client.publish("touch", msg);
  }
  
	long now = millis();
	if (now - lastMsg > 5000)
  {
    lastMsg = now;
    tempESP32 = (temprature_sens_read() - 32) / 1.8;

		String to_send = String(tempESP32);
		to_send.toCharArray(msg, 5);
		client.publish("temp", msg);
	}
  //Serial.println("ESTADO ACTUAL:" + String(estado_led_act) + "\t" + "ESTADO ANTERIOR:" + String(estado_led_last) );
}



//*****************************
//***    CONEXION WIFI      ***
//*****************************
void setup_wifi()
{
	delay(10);
	// Nos conectamos a nuestra red Wifi
	Serial.println();
	Serial.print("Conectando a ");
	Serial.println(ssid);

	WiFi.begin(ssid, password);

	while (WiFi.status() != WL_CONNECTED) 
  {
		delay(500);
		Serial.print(".");
	}

	Serial.println("");
	Serial.println("Conectado a red WiFi!");
	Serial.println("Dirección IP: ");
	Serial.println(WiFi.localIP());
}



void callback(char* topic, byte* payload, unsigned int length)
{
	String incoming = "";
	Serial.print("Mensaje recibido desde -> ");
	Serial.print(topic);
	Serial.println("");
	for (int i = 0; i < length; i++) 
  {
		incoming += (char)payload[i];
	}
	incoming.trim();
	Serial.println("Mensaje -> " + incoming);

	if ( incoming == "on") 
  {
		digitalWrite(BUILTIN_LED, HIGH);
	} else 
  {
		digitalWrite(BUILTIN_LED, LOW);
	}
}

void reconnect() 
{
	while (!client.connected()) 
  {
		Serial.print("Intentando conexión Mqtt...");
		// Creamos un cliente ID
		String clientId = "esp32_";
		clientId += String(random(0xffff), HEX);
		// Intentamos conectar
		if (client.connect(clientId.c_str(),mqtt_user,mqtt_pass)) 
    {
			Serial.println("Conectado!");
			// Nos suscribimos
			client.subscribe("led1");
			client.subscribe("led2");
      client.subscribe("led3");
      client.subscribe("led4");

      String to_send = "APAGADO";
      to_send.toCharArray(msg, 9);
      client.publish("status_led", msg);

      to_send = "DESACTIVADO";
      to_send.toCharArray(msg, 12);
      client.publish("touch", msg);
		} else {
			Serial.print("falló :( con error -> ");
			Serial.print(client.state());
			Serial.println(" Intentamos de nuevo en 5 segundos");

			delay(5000);
		}
	}
}
