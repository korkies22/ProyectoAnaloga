
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>
 
// Data wire is plugged into pin 2 on the Arduino
#define ONE_WIRE_BUS 11
#define MAX_PWM 255
#define MIN_PWM 0
 
// Setup a oneWire instance to communicate with any OneWire devices 
// (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

double temperature;
 
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

int PIN_PWM=10;
int PIN_SISTEMA=A0;
int PIN_ENCENDIDO=A1;
int PIN_LLEGO=A2;

const byte ROWS = 4; 
const byte COLS = 4; 

const float MIN_T=25.0;
const float MAX_T=50.0;

unsigned long tiempoEnvio=0;
unsigned long tiempoSeguir=0;
unsigned long tiempoLCD=0;
boolean mostradoMensajePID=false;
boolean encendido;

float tAct;
float tDeseada;

String entrada="";
boolean hayPunto=false;
boolean escribiendo=false;
boolean llego=false;
boolean porBluetooth=false;
boolean encenderPID=false;

double integral = 0;
double derivador = 0;
double kp = 1;
double ki = 0.1;
double kd = 1;
double pid = 0;
double error_ant = 0;
double error = 0;
double error2 = 0;
double error3 = 0;
double error4 = 0;
double error5 = 0;
double error6 = 0;
double error7 = 0;
double error8 = 0;
double error9 = 0;
double error10 = 0;

byte grados[] = {
  B00111,
  B00101,
  B00111,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000
};

char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {9, 8, 7, 6}; 
byte colPins[COLS] = {5, 4, 3, 2}; 

Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 
LiquidCrystal_I2C lcd(0x27, 16, 2);  

void setup(){
  encendido=false;
  analogWrite(PIN_PWM, 0);
  Serial.begin(9600);

  // Start up the library
  sensors.begin();
  sensors.setResolution(12);
  lcd.init();
  lcd.backlight();
  lcd.createChar(0, grados);

  pinMode(PIN_SISTEMA,OUTPUT);
  pinMode(PIN_ENCENDIDO,OUTPUT);
  pinMode(PIN_LLEGO,OUTPUT);
  digitalWrite(PIN_SISTEMA,HIGH);
  digitalWrite(PIN_ENCENDIDO,LOW);
  digitalWrite(PIN_LLEGO,LOW);

  lcd.home();
  lcd.print("Ingrese Temp");
  lcd.setCursor(0, 1); 
  lcd.print("en ");
  lcd.write(byte(0));
  lcd.print("Centigrados");
}
  
void loop(){
  if(encendido && millis()-tiempoEnvio>5000){
    tiempoEnvio=millis();
    if(llego){
      Serial.println("V:::"+String(tAct));
    }
    else{
       Serial.println("H:::"+String(tAct));
    }
  }
  
  char customKey = customKeypad.getKey();
  if (customKey){
    if(customKey=='A'){
      apagar();
      entrada="";
      hayPunto=false;
    }
    else if(isDigit(customKey)){
      if(hayPunto){
        entrada+=customKey;
        comprobarTemperatura(entrada.toFloat());
        entrada="";
      }
      else if(entrada.length()==2){
        entrada=customKey;
      }
      else{
        entrada+=customKey;
      }
    }
    else if(customKey=='.'){
      if(!hayPunto){
        entrada+=customKey;
        hayPunto=true;
      }
    }
    else if(customKey=='#'){
      comprobarTemperatura(entrada.toFloat());
      entrada="";
      hayPunto=false;
    }
    escribiendo=true;
  }

  if(escribiendo){
    actualizarEscribiendo();
  }

  if(Serial.available() > 0)
  {
    porBluetooth=true;
    char inChar = Serial.read();
    if(inChar == 'T')
    {
       float val= Serial.parseFloat();
       comprobarTemperatura(val);
       if(tDeseada==val){
        Serial.println("H:::"+(String(tAct)));
       }
       else{
        Serial.println('F');
       }
       
    }
    else if(inChar == 'C')
    {
       apagar();
       porBluetooth=false;
    } 
  }

  if(encendido && millis()-tiempoSeguir>400){
    tiempoSeguir=millis();
     convertirTemperatura();
     if(tAct<0){
      return;
     }
     if(abs(tDeseada - tAct)<4){
      encenderPID=true;
      if(tDeseada>tAct){
        integral=(255/ki)/1.2;
      }
      else{
        integral=(-255/ki)/1.2;
      }
     }
     else if(abs(tDeseada - tAct)>10){
      mostradoMensajePID=false;
      error10 =0;
      error9 = 0;
      error8 = 0;
      error7 = 0;
      error6 = 0;
      error5 = 0;
      error4 = 0;
      error3 = 0;
      error2 = 0;
      error_ant = 0;
      error=0;
      integral=0;
      encenderPID=false;
      if(tDeseada>=tAct){
         analogWrite( PIN_PWM, MAX_PWM);
      }
      else{
        analogWrite( PIN_PWM, MIN_PWM);
      }
     }
    if(encenderPID){
      seguir();
    }
    if(encendido){
      digitalWrite(PIN_ENCENDIDO, HIGH); 
    }
    else{
      digitalWrite(PIN_ENCENDIDO, LOW); 
    }
  
    if(llego){
      digitalWrite(PIN_LLEGO, HIGH); 
    }
    else{
      digitalWrite(PIN_LLEGO, LOW); 
    }
  }
  if(millis()-tiempoLCD>1000){
    tiempoLCD=millis();
    reestablecerLCD();
  }
  
}

void comprobarTemperatura(float val){
  if(val<MIN_T || val>MAX_T){
    Serial.println("F");
    lcd.home();
    lcd.print("Rango incorrecto");
    lcd.setCursor(0, 1); 
    lcd.print("Entre ");
    lcd.print(MIN_T,0);
    lcd.print(" y ");
    lcd.print(MAX_T,0);
    lcd.print(" ");
    lcd.write(byte(0));
    lcd.print("C");
    delay(2000);
  }
  else{
    encender(val);
  }
}

void reestablecerLCD(){
  if(!mostradoMensajePID){
    mostradoMensajePID=true;
    lcd.home();
    lcd.print("PWM activado");
    lcd.setCursor(0, 1); 
    lcd.print("Estamos cerca");
    delay(1500);
  }
  else if(encendido){
     lcd.home();
     lcd.print("Actual: ");
     lcd.print(tAct,2);
     lcd.write(byte(0));
     lcd.print("C");
     lcd.setCursor(0, 1);
     lcd.print("Deseada: ");
     lcd.print(tDeseada,2);
     lcd.write(byte(0));
     lcd.print("C");
  }
  else{
    lcd.home();
    lcd.print("Ingrese T");
    lcd.setCursor(0, 1); 
    lcd.print("en ");
    lcd.write(byte(0));
    lcd.print("Centigrados");
  }
}

void actualizarEscribiendo(){
   lcd.home();
   lcd.print("Entre ");
   lcd.print(MIN_T,0);
   lcd.print(" y ");
   lcd.print(MAX_T,0);
   lcd.print(" ");
   lcd.write(byte(0));
   lcd.print("C");
   lcd.setCursor(0, 1); 
   lcd.print("Deseada: ");
   lcd.print(entrada);
   lcd.print(" ");
   lcd.write(byte(0));
   lcd.print("Centigrados");
}

void apagar(){
  encendido = false;
  llego=false;
  encenderPID=false;
  mostradoMensajePID=false;
  analogWrite(PIN_PWM, MIN_PWM);
  Serial.println('C');
}

void encender(float val){
  tDeseada=val;
  encendido=true;
  tiempoSeguir=0;
  convertirTemperatura();
}

void convertirTemperatura(){
  sensors.requestTemperatures(); // Send the command to get temperatures
  tAct=sensors.getTempCByIndex(0);
}

void seguir() {
  //Serial.println("seguir");
  error = tDeseada - tAct;
  if(abs(error)<1 && abs(integral)<2){
    llego=true;
  }
  else{
    llego=false;
  }
  derivador = error - error5;
  integral = integral+error;
  if(ki!=0 && integral>(255/ki)){
    integral=(255/ki);
  }
  if(ki!=0 && integral<(-255/ki)){
    integral=(-255/ki);
  }
  error10 = error9;
  error9 = error8;
  error8 = error7;
  error7 = error6;
  error6 = error5;
  error5 = error4;
  error4 = error3;
  error3 = error2;
  error2 = error_ant;
  error_ant = error;
  pid = kp * error + kd * derivador + ki * integral;
  double valPWM = pid;
  if (valPWM > MAX_PWM)
  {
    valPWM=MAX_PWM;
  }
  else if (valPWM < MIN_PWM)
  {
    valPWM=MIN_PWM;
  }
  analogWrite( PIN_PWM, (int)valPWM);
}
