#include <Arduino.h>

#include <OneWire.h>
#include <DallasTemperature.h>
#include<U8g2lib.h>
#include <WiFi.h>
#define ESPALEXA_MAXDEVICES 1
#include <Espalexa.h>
#include <credentials.h>
#include <Tone32.h>

#define BUZZER_PIN 5
#define BUZZER_CHANNEL 0

#ifdef U8X8_HAVE_HW_SPI
#include<SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include<Wire.h>
#endif


U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0,22, 21,  U8X8_PIN_NONE);

const int oneWireBus = 4;
OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);

const  int Up = 25;
const int Down = 27;
const int Act = 13;
const int ledPin = 5;
bool sound;

int VolSens;
int LvlSens = 14;

int SolShow = 18;
int SolTank = 19;

String Temperatura;
int Mode = 0;

int rep = 0;
int alar = 0;
int a = 0;

struct volumen {
  String LLeno = "lleno";
  String Vacio = "";
};
  volumen Nivel;
  String Capacidad;

struct tank{
  int Vol;
  int lvl;
  bool full = false;
};
  tank Estanque;

struct Estado {
  String Activo = "Activo";
  String Esperando = "esperando";
  String Inactivo = "inactivo";
};
  Estado estado;
  String Status;

struct Buttons {
  int Up;
  int Down;
  int Act;
};
  Buttons button;

struct Temp {
  int ActualTemp;
  int LastTemp;
};
  Temp T;
  int SetTemp;

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASS;

bool connectWifi();

void buzzer(int delayTime, int led, int intervalo) {
    if (sound == false){
    for (int lap = 0; intervalo >= lap; lap++){
        ledcWrite(led, 50 );
        delay(delayTime);
        ledcWrite(led, 0);
        delay(delayTime);
        // Serial.println(lap);
        if (lap >= intervalo){
          sound = true;
      }
    }
  }
    else{
      ledcWrite(led, 0);
    }
  }

void DuchaChanged(EspalexaDevice* dev);

bool wifiConnected = false;

Espalexa espalexa;
void getTemperature (){
   T.ActualTemp = sensors.getTempCByIndex(0);
}

void DuchaChanged(EspalexaDevice* d) {
  if (d == nullptr) return;
  uint8_t degrees = d->getDegrees(); //for heaters, HVAC, ...
  SetTemp = degrees;
}

void inicio(){
  tone(BUZZER_PIN, NOTE_FS4, 250, BUZZER_CHANNEL);
  noTone(BUZZER_PIN, BUZZER_CHANNEL);
  tone(BUZZER_PIN, NOTE_A4, 125, BUZZER_CHANNEL);
  noTone(BUZZER_PIN, BUZZER_CHANNEL);
  tone(BUZZER_PIN, NOTE_B4, 125, BUZZER_CHANNEL);
  noTone(BUZZER_PIN, BUZZER_CHANNEL);
  tone(BUZZER_PIN, NOTE_CS5, 250, BUZZER_CHANNEL);
  noTone(BUZZER_PIN, BUZZER_CHANNEL);
  tone(BUZZER_PIN, NOTE_A4, 64, BUZZER_CHANNEL);
  noTone(BUZZER_PIN, BUZZER_CHANNEL);
  tone(BUZZER_PIN, NOTE_E5, 1500, BUZZER_CHANNEL);
  noTone(BUZZER_PIN, BUZZER_CHANNEL);
}
void final(){
  tone(BUZZER_PIN, NOTE_CS5, 250, BUZZER_CHANNEL);
  noTone(BUZZER_PIN, BUZZER_CHANNEL);
  tone(BUZZER_PIN, NOTE_E5, 125, BUZZER_CHANNEL);
  noTone(BUZZER_PIN, BUZZER_CHANNEL);
  tone(BUZZER_PIN, NOTE_B4, 125, BUZZER_CHANNEL);
  noTone(BUZZER_PIN, BUZZER_CHANNEL);
  tone(BUZZER_PIN, NOTE_CS5, 125, BUZZER_CHANNEL);
  noTone(BUZZER_PIN, BUZZER_CHANNEL);
  tone(BUZZER_PIN, NOTE_A4, 125, BUZZER_CHANNEL);
  noTone(BUZZER_PIN, BUZZER_CHANNEL);
  tone(BUZZER_PIN, NOTE_FS4, 1500, BUZZER_CHANNEL);
  noTone(BUZZER_PIN, BUZZER_CHANNEL);
}
void error(){
  tone(BUZZER_PIN, NOTE_B3, 3000, BUZZER_CHANNEL);
  noTone(BUZZER_PIN, BUZZER_CHANNEL);
  delay(10000);
}

void setup(){
  Serial.begin(115200);
  sensors.begin();
  u8g2.begin();
  wifiConnected = connectWifi();
  espalexa.addDevice("ducha", DuchaChanged, EspalexaDeviceType::dimmable, 127);
  espalexa.begin();

  pinMode(VolSens, INPUT);
  pinMode(LvlSens, INPUT_PULLUP);

  pinMode(SolShow, OUTPUT);
  pinMode(SolTank, OUTPUT);
  pinMode(VolSens, OUTPUT);

  pinMode(Up, INPUT_PULLUP);
  pinMode(Down, INPUT_PULLUP);
  pinMode(Act, INPUT_PULLUP);
}

void loop(){
 sensors.requestTemperatures();
 getTemperature ();
 button.Up = digitalRead(Up);
 button.Down = digitalRead(Down);
 button.Act = digitalRead(Act);
 Estanque.lvl= digitalRead(LvlSens);
 espalexa.loop();
 delay(1);

 u8g2.firstPage();

 if (SetTemp != 20){
   Mode = 1;
   a = 0;
 }
 if (Mode == 0){
   Status = estado.Inactivo;
   digitalWrite(SolShow, HIGH);
   digitalWrite(SolTank, HIGH);
 }
 if (button.Act == LOW){
   Mode = Mode + 1;
   delay (500);
   sound = false;
 }

 if (T.ActualTemp < SetTemp && Mode != 0 && Estanque.full == false){
   digitalWrite(SolShow, HIGH);
   digitalWrite(SolTank, LOW);
   Status = estado.Esperando;
   rep = 0;
 }
 if (T.ActualTemp >= SetTemp && Mode != 0 && Estanque.full == false){
   digitalWrite(SolShow, LOW);
   digitalWrite(SolTank, HIGH);
   Status = estado.Activo;
   sound =false;
   if (rep == 0){
     inicio();
     rep = rep +1;
   }
 }
 if( Mode >= 2 && button.Act == LOW || SetTemp == 20){
   Mode = 0;
   SetTemp= 20;

   if (a==0){
     final();
     a = a+1;
   }
   delay (500);
 }
 if (Estanque.lvl == LOW){
   digitalWrite(SolShow, LOW);
   digitalWrite(SolTank, HIGH);
   Estanque.full = true;
   Capacidad = Nivel.LLeno;
   sound =false;

   }
 else {
   Capacidad = Nivel.Vacio;
   Estanque.full = false;
   alar = 0;
 }
 if(Estanque.full == true){
   if (alar == 0){
     error();
     alar = alar +1;
   }
 }
 if (Estanque.full == true && Mode == 0){
   digitalWrite(SolShow, LOW);
   digitalWrite(SolTank, LOW);

 }
 if (T.ActualTemp < SetTemp && Mode != 0 && Estanque.full == true){
   digitalWrite(SolShow, LOW);
   digitalWrite(SolTank, HIGH);
   Status = estado.Esperando;
   rep = 0;
 }
 if (T.ActualTemp >= SetTemp && Mode != 0 && Estanque.full == true){
   digitalWrite(SolShow, LOW);
   digitalWrite(SolTank, HIGH);
   Status = estado.Activo;
   sound =false;
   if (rep == 0){
    inicio();
     rep = rep +1;
   }
 }
 if (button.Up == LOW){
   if(SetTemp == 20){
     SetTemp = 25;
   }
   SetTemp = SetTemp + 1;
   tone(BUZZER_PIN, NOTE_A4, 125, BUZZER_CHANNEL);
   noTone(BUZZER_PIN, BUZZER_CHANNEL);
   // Serial.println(SetTemp);
   delay(250);
   }
 if (button.Down == LOW){

   SetTemp = SetTemp - 1;
   tone(BUZZER_PIN, NOTE_E4, 125, BUZZER_CHANNEL);
   noTone(BUZZER_PIN, BUZZER_CHANNEL);
   // Serial.println(SetTemp);
   delay(250);
   }
   if(SetTemp <= 21){
     SetTemp = 20;
   }
 do {
   u8g2.setFont(u8g2_font_ncenB08_tr);
   u8g2.drawStr(0,15,"T Set");
   u8g2.setCursor(30, 15);
   u8g2.print(SetTemp);
   u8g2.drawStr(48,15,"C");
   u8g2.setFont(u8g2_font_unifont_t_symbols);
   u8g2.drawGlyph(40, 15, 0x00b0);
   ////////////////////////////////
   u8g2.setFont(u8g2_font_ncenB08_tr);
   u8g2.drawStr(64,15,"T Act");
   u8g2.setCursor(94, 15);
   u8g2.print(T.ActualTemp);
   u8g2.drawStr(113, 15,"C");
   u8g2.setFont(u8g2_font_unifont_t_symbols);
   u8g2.drawGlyph(104, 15, 0x00b0);
   ///////////////////////////////
   int largo = ((Status.length()+(Status.length()*6)/2));
   int center = (63 - largo);
   u8g2.setFont(u8g2_font_ncenB10_tr);
   u8g2.setCursor(center, 35);
   u8g2.print(Status);
   ////////////////////////////////
   int largo1 = ((Capacidad.length()+(Capacidad.length()*6)/2));
   int center1 = (63 - largo1);
   u8g2.setFont(u8g2_font_ncenB10_tr);
   u8g2.setCursor(center1, 55);
   u8g2.print(Capacidad);
 }
 while ( u8g2.nextPage() );
}
bool connectWifi(){
  bool state = true;
  int i = 0;

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  // Serial.println("");
  // Serial.println("Connecting to WiFi");

  // Wait for connection
  // Serial.print("Connecting...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (i > 20){
      state = false; break;
    }
    i++;
  }
  // Serial.println("");
  if (state){
    // Serial.print("Connected to ");
    // Serial.println(ssid);
    // Serial.print("IP address: ");
    // Serial.println(WiFi.localIP());
  }
  else {
    // Serial.println("Connection failed.");
  }
  return state;
}
