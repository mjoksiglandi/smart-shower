#include <OneWire.h>
#include <DallasTemperature.h>
#include<U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include<SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include<Wire.h>
#endif

U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* clock=*/ 22, /* data=*/ 21, /* reset=*/ U8X8_PIN_NONE);

const int oneWireBus = 4;
OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);

const  int Up = 25;
const int Down = 27;
const int Act = 13;
const int ledPin = 5;
bool sound;

 int freq = 440;
 int ledChannel = 0;
 int resolution = 10;

int VolSens;
int LvlSens = 14;

int SolShow = 18;
int SolTank = 19;

String Temperatura;
int Mode = 0;

int rep = 0;
int alar = 0;

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
  int SetTemp = 27;

  void buzzer(int delayTime, int led, int intervalo) {
    if (sound == false){
    for (int lap = 0; intervalo >= lap; lap++){
        ledcWrite(led, 50 );
        delay(delayTime);
        ledcWrite(led, 0);
        delay(delayTime);
        Serial.println(lap);
        if (lap >= intervalo){
          sound = true;
      }
    }
  }
    else{
      ledcWrite(led, 0);
    }
  }

void setup() {
  Serial.begin (115200);
  sensors.begin();
  u8g2.begin();

  ledcSetup(ledChannel, freq, resolution);

  pinMode(VolSens, INPUT);
  pinMode(LvlSens, INPUT_PULLUP);

  pinMode(SolShow, OUTPUT);
  pinMode(SolTank, OUTPUT);
  pinMode(VolSens, OUTPUT);

  pinMode(Up, INPUT_PULLUP);
  pinMode(Down, INPUT_PULLUP);
  pinMode(Act, INPUT_PULLUP);

  ledcAttachPin(ledPin, ledChannel);

  Serial.begin(115200);
  buzzer(50,  ledChannel, 3);
}

void loop() {
  sensors.requestTemperatures();
  T.ActualTemp = sensors.getTempCByIndex(0);
  button.Up = digitalRead(Up);
  button.Down = digitalRead(Down);
  button.Act = digitalRead(Act);
  Estanque.lvl= digitalRead(LvlSens);

  u8g2.firstPage();

  Serial.println(rep);

  if (Mode == 0){
    Status = estado.Inactivo;
    digitalWrite(SolShow, HIGH);
    digitalWrite(SolTank, HIGH);
  }
  if (button.Act == LOW){
    Mode = Mode + 1;
    delay (500);
    sound = false;
    buzzer(200,  ledChannel, 0);
  }

  if (T.ActualTemp < SetTemp && Mode != 0 && Estanque.full == false){
    digitalWrite(SolShow, LOW);
    digitalWrite(SolTank, HIGH);
    Status = estado.Esperando;
    rep = 0;
  }
  if (T.ActualTemp >= SetTemp && Mode != 0 && Estanque.full == false){
    digitalWrite(SolShow, HIGH);
    digitalWrite(SolTank, LOW);
    Status = estado.Activo;
    Serial.println(rep);
    sound =false;
    if (rep == 0){
      buzzer(500,  ledChannel, 1);
      rep = rep +1;
    }
  }
  if( Mode >= 2 && button.Act == HIGH){
    Mode = 0;
    delay (500);
  }
  if (Estanque.lvl == LOW){
    digitalWrite(SolShow, LOW);
    digitalWrite(SolTank, LOW);
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
      buzzer(1000,  ledChannel, 3);
      alar = alar +1;
    }
    // buzzer(1000,  ledChannel, 6);
  }
  if (T.ActualTemp < SetTemp && Mode != 0 && Estanque.full == true){
    digitalWrite(SolShow, LOW);
    digitalWrite(SolTank, LOW);
    Status = estado.Esperando;
    rep = 0;
  }
  if (T.ActualTemp >= SetTemp && Mode != 0 && Estanque.full == true){
    digitalWrite(SolShow, LOW);
    digitalWrite(SolTank, LOW);
    Status = estado.Activo;
    sound =false;
    if (rep == 0){
      buzzer(500,  ledChannel, 1);
      rep = rep +1;
    }
  }
  if (button.Up == LOW){
    SetTemp = SetTemp + 1;
    Serial.println(SetTemp);
    delay(250);
    }
  if (button.Down == LOW){
    SetTemp = SetTemp - 1;
    Serial.println(SetTemp);
    delay(250);
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