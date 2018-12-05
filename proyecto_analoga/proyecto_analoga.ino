#include <Keypad.h>
#include <LiquidCrystal_I2C.h>

int PIN_PWM=10;
int PIN_LECTURA=0;
int PIN_ENCENDIDO=10;
int PIN_LLEGO=11;

const byte ROWS = 4; 
const byte COLS = 4; 

const float MIN_T=25.0;
const float MAX_T=50.0;

unsigned long tiempoEnvio=0;
unsigned long tiempoSeguir=0;
boolean encendido;

float tAct;
float tDeseada;

String entrada="";
boolean hayPunto=false;
boolean escribiendo=false;
boolean llego=false;
boolean porBluetooth=false;

double integral = 0;
double derivador = 0;
double kp = 0.1;
double ki = 0.001;
double kd = 0.01;
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
  lcd.init();
  lcd.backlight();
  lcd.createChar(0, grados);

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
        Serial.println("H"+:::+(String(tAct));
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

  if(encendido && millis()-tiempoSeguir>1000){
    seguir();
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
  reestablecerLCD();
}

void reestablecerLCD(){
  if(encendido){
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
  analogWrite(PIN_PWM, 0);
  Serial.println('C');
  reestablecerLCD();
}

void encender(float val){
  tDeseada=val;
  encendido=true;
  tiempoSeguir=0;
  convertirTemperatura();
}

void convertirTemperatura(){
  int valorT= analogRead(PIN_LECTURA);
  tAct= 0.0475*valorT+13.668;
}

void seguir() {
  convertirTemperatura();
  //Serial.println("seguir");
  error = tDeseada - tAct;
  if(abs(error)<1){
    llego=true;
  }
  else{
    llego=false;
  }
  derivador = error - error5;
  integral = error_ant + error2 + error3 + error4 + error5 + error6 + error7 + error8 + error9 + error10 + error;
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
  if (valPWM > 255)
  {
    valPWM=255;
  }
  else if (valPWM < 0)
  {
    valPWM=0;
  }
  analogWrite( PIN_PWM, (int)valPWM);
}
