#include <Doomsday8266.hpp>
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_PIN 13
#define PIN_1 12
#define PIN_2 14
#define PIN_3 16

OneWire ds(ONE_WIRE_PIN);
byte addr[8];
byte type_s;
DallasTemperature ds1820(&ds);

Trigger<0>* boil = new Trigger<0>("BOIL");
Trigger<0>* standby = new Trigger<0>("STANDBY");
Trigger<1>* heat = new Trigger<1>("HEAT", new IntParam(30, 95));
IntSensor* temperature = new IntSensor("TEMPERATURE", 0, 100);
IntSensor* waterLevel = new IntSensor("WATER", 0, 3);
ValSensor<3>* workMode = new ValSensor<3>("MODE", "STANDBY", "BOIL", "HEAT");

int mode = 0;
int currentTemp = 0;
int heatTarget = 0;
int level = 0;
bool heatPaused = true;

void onBoil(TArg arg) {
  setMode(1);
}

void onStandby(TArg arg) {
  setMode(0);
}

void onHeat(TArg arg) {
  setMode(2);
  heatTarget = arg.asInt(0);
}

void setup(Endpoint* e) {
  pinMode(5, OUTPUT);
  digitalWrite(5, LOW);
  
  pinMode(4, INPUT_PULLUP);
  
  ds1820.begin();

  pinMode(PIN_1, OUTPUT);
  digitalWrite(PIN_1, LOW);
  pinMode(PIN_2, OUTPUT);
  digitalWrite(PIN_2, LOW);
  pinMode(PIN_3, OUTPUT);
  digitalWrite(PIN_3, LOW);

  

  boil->on(onBoil);
  standby->on(onStandby);
  heat->on(onHeat);
  e->addTrigger(boil);
  e->addTrigger(standby);
  e->addTrigger(heat);
  e->addSensor(temperature);
  e->addSensor(waterLevel);
  e->addSensor(workMode);
  e->setDevClass("TEAPOT.DDD-DIY.1");
}

void setMode(int m) {
  workMode->set(m);
  mode = m;
}

void powerOn() {
  digitalWrite(5, HIGH);
}

void powerOff() {
  digitalWrite(5, LOW);
}


float getTemp() {
  ds1820.requestTemperatures();
  delay(100);
  float t = ds1820.getTempCByIndex(0);
  return t;
}

bool checkPin(int pin) {
  digitalWrite(pin, HIGH);
  delay(20);
  int v = analogRead(A0);
  delay(10);
  digitalWrite(pin, LOW);
  return v > 190;
}

int checkLevel() {
  if (checkPin(PIN_3)) return 3;
  if (checkPin(PIN_2)) return 2;
  if (checkPin(PIN_1)) return 1;
  return 0;
}

void boilLoop() {
  if (currentTemp >= 98) {
    setMode(0);
    powerOff();
  } else {
    powerOn();
  }
}

void heatLoop() {
  if (heatPaused) {
    if (currentTemp < heatTarget) {
      heatPaused = false;
      powerOn();
    }
  } else {
    if (currentTemp >= heatTarget) {
      heatPaused = true;
      powerOff();
    }
  }
}

void checkButton(){
  if (digitalRead(4)==LOW){
    setMode(1);
  }
}

void loop(Endpoint* e) {
  checkButton();
  
  currentTemp = getTemp();
  level = checkLevel();
  temperature->set(currentTemp);
  waterLevel->set(level);

  if  (level == 0) {
    setMode(0);
    powerOff();
  }


  switch (mode) {
    case 0: powerOff(); break;
    case 1: boilLoop(); break;
    case 2: heatLoop(); break;
  }
}
