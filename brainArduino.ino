#include <ArduinoJson.h>

//#define DEBUG_ARRAY(a) {for (int index = 0; index < sizeof(a) / sizeof(a[0]); index++)    {Serial.print(a[index]); Serial.print('\t');} Serial.println();};
#define DEBUG(a, b) for (int index = 0; index < b; index++) Serial.print(a[index]); Serial.println();

String str = "";
const char separator = ':';
const int dataLength = 2;
int data[dataLength];


const size_t bufferSizeWrite = 4 * JSON_ARRAY_SIZE(4) + JSON_ARRAY_SIZE(5) + JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(6);
const size_t bufferSizeRead = JSON_ARRAY_SIZE(3) + JSON_OBJECT_SIZE(8) + 100;
//----------------------------------------------------
#include <DHT.h>
#define DHTPIN A0
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

#include <SoftwareSerial.h>

//#include <String.h>

#include <LiquidCrystal.h>

#include "Wire.h"
#define DS1307_ADDRESS 0x68
byte zero = 0x00; //workaround for issue #527

byte minute;
byte hour;
byte weekDay;
byte monthDay;
byte month;
byte year;


//#include "RTClib.h"

//RTC_DS1307 RTC;


int modulesCount = 4; // Num modules
int cycleNumber = 1;

SoftwareSerial serialGPRS(50, 51); // Serial GRPS
LiquidCrystal lcd(8, 9, 4, 5, 6, 7); // LCD: ( RS, E, LCD-D4, LCD-D5, LCD-D6, LCD-D7 )

// Variables temporales para medici�n de promedio de mediciones.
int temporaltemperature[4];
int temporalhumidity[4];
int temporalammonia[4];

// Almacenamiento de Entradas de datos
int temperature[4]; // Port A0 IN
int humidity[4]; // Port A0 IN
int ammonia[4]; // Port A1 IN
int fire[4]; // Port D4 IN
float light; // Port A2 IN
int rain; // Port D5 IN

//buzzer alertas
int buzzer = 10;

//Buttons LCD
int B;
int inbuttonlcd = A2; // entrada/analoga botones lcd

// LLave JSON, informaci�n de sensores.
//String keys[]={"tempearture:", "humidity:", "ammonia:","fire:", "light:","rain:"};

// Puertos de Entradas de datos - digital
int inFire = 44;
int inRain = 53;

// LED RESERVA
//int reserva=31; // led de reserva por si toca utilizarlo

// Conirmacion alarma
//int inStopAlarm = 28; // Pulsador para parar las llamadas / S-IN Stop in
int outStopAlarm = 26; // Led de confirmacion / S-OUT Stop in

// LED manual y Auto
int led_m_a = 30;

// Puertos de Entradas de datos - analogic
int inAmmonia = A1;
int inLight = A15;

// Puertos de Salida Selectores
int Sen_SelectA = 46;
int Sen_SelectB = 48;

// Puertos de Salida Selectores Actuadores
int Act_SelectA = 35;
int Act_SelectB = 33;

// actuadores manuales
int manualLight = 43; // High Si el boton deluces es presionado
int manualAbanico = 41; // High Si el boton de abanico

// actuadores automatico
int autoAbanico = 37; // AUTO: High if Temperature > 33�, Low if Temperature < 31�
int autoLight = 39; //Hight if 5:30 Pm to 7:30 Pm LOS DOS EN EL 29 A QUIEN LE CREO?

// Confirmacion LED luz y abanico
int luz_led = 24; // confirmacion predido y pagado luz
int aba_led = 22; // confirmacion predido y pagado abanico

// LED fallos sensores
int led_fallos = 28;

int inAbanico = 45; //  High si la salida al actuador avanico es HIGH
int inLuz = 47; // Lee salida luces multiplexada

// Puertos de Salida Actuadores
//int outLight = 40; //Hight if 5:30 Pm to 7:30 Pm LOS DOS EN EL 29 A QUIEN LE CREO?

int outLight = 31;
int outAbanico = 29; // High if Temperature > 33�, Low if Temperature < 31�
int outAbanico2 = 27; // High if Temperature > 33�, Low if Temperature < 31�
int outAbanico3 = 25; // High if Temperature > 33�, Low if Temperature < 31�

int outControlGprs = 49;

// Reference values
int maxTemp = 100, minTemp = 0,
    maxHum = 100, minHum = 0,
    maxAir = 1000;
//String number="3003162831";

// Default States
boolean stopAlarmStatus = false;
boolean Alarms[8];
boolean lecture_readbuttons = false;
boolean auto_mode = true; // Modo automatico por defecto
boolean manual_mode = false;
boolean abanico_ON = false;
boolean luz_ON = false;
boolean abanico_2_3_ON = false;
boolean lluvia_confirmacion = false, fuego_confirmacion = false, temp_confirmacion = false, ammonia_confirmacion = false, humedad_confirmacion = false;
boolean fallo = false;

void setup() {

  Serial.begin(19200); // the GPRS baud rate

  delay(500);

  //Serial.println("Iniciando Programa!!!.......");

  //RTC.begin(); // Inicia la comunicaci�n con el RTC
  dht.begin();  // Iniciaqndo DTH11
  Wire.begin();  // Comunicacion por un cabe RTC
  serialGPRS.begin(19200); // the GPRS baud rate

  delay(500);

  pinMode(led_fallos, OUTPUT); // Salida led fallos sensores

  pinMode(buzzer, OUTPUT); // In Rain
  pinMode(inRain, INPUT); // In Rain
  pinMode(inFire, INPUT); // In Rain
  pinMode(led_m_a, OUTPUT); // In StopAlarm
  pinMode(outStopAlarm, OUTPUT);

  pinMode(luz_led, OUTPUT);
  pinMode(aba_led, OUTPUT);

  pinMode(inAmmonia, INPUT); // In Ammonia
  pinMode(inLight, INPUT); // In Light

  pinMode(Sen_SelectA, OUTPUT); // Output SelectA
  pinMode(Sen_SelectB, OUTPUT); // Output SelectB

  pinMode(Act_SelectA, OUTPUT); // Output SelectA  Sensores
  pinMode(Act_SelectB, OUTPUT); // Output SelectB Sensores

  pinMode(manualAbanico, OUTPUT); // Output Auto abanico
  pinMode(manualLight, OUTPUT);// Salida Luz manual

  pinMode(autoLight, OUTPUT); // Output auto Light    pinMode(outmanualLight, OUTPUT); // Output manual Light
  pinMode(autoAbanico, OUTPUT); // Output Auto abanico

  pinMode(inAbanico, INPUT); // Output Auto abanico
  pinMode(inLuz, INPUT); // Output Auto abanico

  pinMode(outLight, OUTPUT); // Output Luz
  pinMode(outAbanico, OUTPUT); // Output Abanico2
  pinMode(outAbanico2, OUTPUT); // Output Abanico2
  pinMode(outAbanico3, OUTPUT); // Output Abanico3

  pinMode(outControlGprs, OUTPUT); // Salida prender GPRS

  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("Bienvenido");
  lcd.setCursor(3, 1);
  lcd.print("CSA SYSTEM");



  // Default Output
  digitalWrite(outStopAlarm, LOW);

  digitalWrite(Sen_SelectA, LOW);  // Selectores de sensores en posicion Xo Yo
  digitalWrite(Sen_SelectB, LOW);

  digitalWrite(luz_led, LOW);
  digitalWrite(aba_led, LOW);

  digitalWrite(autoLight, LOW);
  digitalWrite(manualLight, LOW); // Todas las se�ales apagadas
  digitalWrite(autoAbanico, LOW);
  digitalWrite(manualAbanico, LOW);

  digitalWrite(outLight, HIGH);
  digitalWrite(outAbanico, HIGH); // Todos los actuadores apagados
  digitalWrite(outAbanico2, HIGH);
  digitalWrite(outAbanico3, HIGH);

  Alarms[1] = false;
  Alarms[2] = false;
  Alarms[3] = false;
  Alarms[4] = false;
  Alarms[5] = false;
  Alarms[0] = false;
  Alarms[6] = false;
  Alarms[7] = false;

  powerUpOrDown(); // Enciende GPRS module
  delay(2000); // MODIFICAR EL RETARDO CUANDO SE IMPLEMENTE ENVIO DE DATOS!!!

  //Serial.println ("Configurando GPRS SHIELD.........");
  /*
    //Serial.println("INICIANDO GPRS");
    delay (5000);

    serialGPRS.println("AT+CSQ");
    delay(100);

    ShowSerialData();// this code is to show the data from gprs shield, in order to easily see the process of how the gprs shield submit a http request, and the following is for this purpose too.

    serialGPRS.println("AT+CGATT?");
    delay(100);

    ShowSerialData();

    serialGPRS.println("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"");//setting the SAPBR, the connection type is using gprs
    delay(1000);

    ShowSerialData();

    serialGPRS.println("AT+SAPBR=3,1,\"APN\",\"web.colombiamovil.com.co\"");//setting the APN, the second need you fill in your local apn server
    delay(4000);
    ShowSerialData();

    serialGPRS.println("AT+SAPBR=3,1,\"USER\",\"tigointeractivo\"");//setting the APN, the second need you fill in your local apn server
    delay(4000);

    ShowSerialData();

    serialGPRS.println("AT+SAPBR=3,1,\"PWD\",\"tigo\"");//setting the APN, the second need you fill in your local apn server
    delay(4000);

    ShowSerialData();

    serialGPRS.println("AT+SAPBR=1,1");//setting the SAPBR, for detail you can refer to the AT command mamual
    delay(2000);

    ShowSerialData();
  */




  // Prueba leds caja
  digitalWrite(led_fallos, HIGH); // boton sensores
  delay (100);
  digitalWrite(led_m_a, HIGH);  // led manual auto
  delay(100);
  digitalWrite(outStopAlarm, HIGH); // boton alerta
  delay (100);
  digitalWrite(aba_led, HIGH);
  delay (100);
  digitalWrite(luz_led, HIGH);
  delay (100);

  digitalWrite(luz_led, LOW);
  delay (100);
  digitalWrite(aba_led, LOW);
  delay (100);
  digitalWrite(outStopAlarm, LOW); // boton alerta
  delay (100);
  digitalWrite(led_m_a, LOW);  // led manual auto
  delay(100);
  digitalWrite(led_fallos, LOW); // boton sensores
  delay (100);

  // Prueba leds caja
  digitalWrite(led_fallos, HIGH); // boton sensores
  delay (100);
  digitalWrite(led_m_a, HIGH);  // led manual auto
  delay(100);
  digitalWrite(outStopAlarm, HIGH); // boton alerta
  delay (100);
  digitalWrite(aba_led, HIGH);
  delay (100);
  digitalWrite(luz_led, HIGH);
  delay (100);

  digitalWrite(luz_led, LOW);
  delay (100);
  digitalWrite(aba_led, LOW);
  delay (100);
  digitalWrite(outStopAlarm, LOW); // boton alerta
  delay (100);
  digitalWrite(led_m_a, LOW);  // led manual auto
  delay(100);
  digitalWrite(led_fallos, LOW); // boton sensores
  delay (100);

  delay(800);

  // Prueba leds caja
  digitalWrite(led_fallos, HIGH); // boton sensores
  digitalWrite(led_m_a, HIGH);  // led manual auto
  digitalWrite(outStopAlarm, HIGH); // boton alerta
  digitalWrite(aba_led, HIGH);
  digitalWrite(luz_led, HIGH);
  delay (500);

  // Prueba leds caja
  digitalWrite(led_fallos, LOW); // boton sensores
  digitalWrite(led_m_a, LOW);  // led manual auto
  digitalWrite(outStopAlarm, LOW); // boton alerta
  digitalWrite(aba_led, LOW);
  digitalWrite(luz_led, LOW);
  delay (100);


  delay(3500);
  //setDateTime();

  //---------------------------------------VOID LOOP----------------------INICIO VOID LOOP--------------------
}

void loop() {


  int rain = digitalRead(inRain);
  //Serial.print("Rain: ");
  //Serial.println(rain);
  int luz = analogRead (inLight);
  //Serial.print("Luz: ");
  //Serial.println(luz);
  //Serial.println(" ");


  //Serial.println("   PrintTimeLcd();");
  PrintTimeLcd();
  //Serial.println(" ");


  //Serial.println(" ReadSensor() ");
  ReadSensor();
  //Serial.println(" ");

  rain = digitalRead(inRain);
  //Serial.print("Rain: ");
  //Serial.println(rain);
  luz = analogRead (inLight);
  //Serial.print("Luz: ");
  //Serial.println(luz);
  //Serial.println(" ");

  //Serial.println(" Buttonslcd() ");
  Buttonslcd();
  //Serial.println(" ");


  rain = digitalRead(inRain);
  //Serial.print("Rain: ");
  //Serial.println(rain);
  luz = analogRead (inLight);
  //Serial.print("Luz: ");
  //Serial.println(luz);
  //Serial.println(" ");

  //Serial.println(" Buttonslcd()");
  Buttonslcd();
  //Serial.println(" ");

  //Serial.println(" Alertas()");
  //Alertas();
  //Serial.println(" ");

  rain = digitalRead(inRain);
  //Serial.print("Rain: ");
  //Serial.println(rain);
  luz = analogRead (inLight);
  //Serial.print("Luz: ");
  //Serial.println(luz);
  //Serial.println(" ");

  //Serial.println("  confir_alerta () ");
  //confir_alerta ();
  //Serial.println(" ");

  rain = digitalRead(inRain);
  //Serial.print("Rain: ");
  //Serial.println(rain);
  luz = analogRead (inLight);
  //Serial.print("Luz: ");
  //Serial.println(luz);
  //Serial.println(" ");

  //Serial.println("fallos (); ");
  fallos ();
  //Serial.println(" ");

  //Serial.println("Auto_Actuadores(); ");
  Auto_Actuadores();
  //Serial.println(" ");

  //Serial.println("  Actuadores_final(); ");
  Actuadores_final();
  //Serial.println(" ");

  //Serial.println("Buttonslcd();");
  Buttonslcd();
  //Serial.println(" ");
  SendJson();

  readData ();

  //Llamada_cel();
  /*
    if(cycleNumber==10){

      for (int i = 0; i < modulesCount; i++)
      {
        temporaltemperature[i] = (temporaltemperature[i] + temperature[i])/cycleNumber;
        temporalhumidity[i] = (temporalhumidity[i] + humidity[i])/cycleNumber;
        temporalammonia[i] = (temporalammonia[i] + ammonia[i])/cycleNumber;
      }

      cycleNumber=1;

      SubmitHttpRequest();

    }else{
      cycleNumber++;
      for (int i = 0; i < modulesCount; i++)
      {
        temporaltemperature[i] = temporaltemperature[i] + temperature[i];
        temporalhumidity[i] = temporalhumidity[i] + humidity[i];
        temporalammonia[i] = temporalammonia[i] + ammonia[i];
      } // FIN FOR
    } // FIN ELSE
  */
}
//---------------------------------------------FIN VOID LOOP----------------------
void SendJson() {

  int temp_0 = temperature[0],
      temp_1 = temperature[1],
      temp_2 = temperature[2],
      temp_3 = temperature[3],
      hum_0 = humidity[0],
      hum_1 = humidity[1],
      hum_2 = humidity[2],
      hum_3 = humidity[3],
      fire_0 = fire[0],
      fire_1 = fire[1],
      fire_2 = fire[2],
      fire_3 = fire[3];

  DynamicJsonBuffer jsonBuffer(bufferSizeWrite);

  JsonObject& root = jsonBuffer.createObject();

  JsonArray& temperature = root.createNestedArray("temperature");
  temperature.add(temp_0);
  temperature.add(temp_1);
  temperature.add(temp_2);
  temperature.add(temp_3);


  JsonArray& humidity = root.createNestedArray("humidity");
  humidity.add(hum_0);
  humidity.add(hum_1);
  humidity.add(hum_2);
  humidity.add(hum_3);

  JsonArray& airQuality = root.createNestedArray("airQuality");
  airQuality.add(ammonia[0]);
  airQuality.add(ammonia[1]);
  airQuality.add(ammonia[2]);
  airQuality.add(ammonia[3]);

  JsonArray& fire = root.createNestedArray("fire");
  fire.add(fire_0);
  fire.add(fire_1);
  fire.add(fire_2);
  fire.add(fire_3);

  JsonObject& others = root.createNestedObject("others");
  others["rain"] = rain;
  others["light"] = light;

  JsonArray& actuadores = root.createNestedArray("actuadores");
  actuadores.add(luz_ON);
  actuadores.add(abanico_ON);
  actuadores.add(false);
  actuadores.add(false);

  int size_json = root.measureLength();
  char jsonChar[size_json];
  root.printTo((char*)jsonChar, size_json + 1);
  Serial.println (jsonChar);
}

void readData () {

  if (Serial.available() > 0) {

    str = Serial.readStringUntil('\n');
    //str = Serial.readString();
    //Serial.println(str);

    DynamicJsonBuffer jsonBuffer(bufferSizeRead);

    JsonObject& root = jsonBuffer.parseObject(str);

    int maxTemp = root["aT"];
    int minTemp = root["iT"];
    int maxHum = root["aH"];
    int minHum = root["iH"];
    int maxAir = root["aA"];

    JsonArray& actuators = root["A"];

    int light = actuators[0];
    int fan = actuators[1];
    int actuators2 = actuators[2];
    int actuators3 = actuators[3];

/*
    Serial.println(maxTemp);
    Serial.println(minTemp);
    Serial.println(maxHum);
    Serial.println(minHum);
    Serial.println(maxAir);

    Serial.println(light);
    Serial.println(fan);
    Serial.println(actuators2);
    Serial.println(actuators3);
*/
    //String data = Serial.readStringUntil('\n');
    //DEBUG(data);
    /* for (int i = 0; i < dataLength ; i++) {
       int index = str.indexOf(separator);
       data[i] = str.substring(0, index).toInt();
       str = str.substring(index + 1);
      }*/
    //DEBUG_ARRAY(data);
    //Serial.println(data[0]);
    //Serial.println(data[1]);

    //int lightVariable = data[0];
    //int fanVariable = data[1];

    if (fan == 1 && abanico_ON == false) {
      Abanico_ON();
      digitalWrite(autoAbanico, HIGH);
      Actuadores_final();
      abanico_ON = true;
    }

    if (fan == 0 && abanico_ON == true) {
      Abanico_OFF();
      digitalWrite(autoAbanico, LOW);
      Actuadores_final();
      abanico_ON = false;
    }

    if (light == 1 && luz_ON == false) {
      Luces_ON();
      digitalWrite(luz_led, HIGH);
      digitalWrite(autoLight, HIGH);
      Actuadores_final();
      luz_ON = true;
    }

    if (light == 0 && luz_ON == true) {
      Luces_OFF();
      digitalWrite(luz_led, LOW);
      digitalWrite(autoLight, LOW);
      Actuadores_final();
      luz_ON = false;
    }

  }

}
//----------------------------VOID  Read Sensor----------------------------VOID  Read Sensor----------------------------VOID  Read Sensor----------------------------

void ReadSensor() {

  //Read one module per second

  for (int i = 0; i < modulesCount; i++) {

    if (i == 0) {

      digitalWrite(Sen_SelectA, LOW);
      digitalWrite(Sen_SelectB, LOW);
      delay(1100);

      temperature[0] = dht.readTemperature();
      humidity[0] = dht.readHumidity();
      ammonia[0] = analogRead(inAmmonia);
      fire[0] = digitalRead(inFire);
      light = analogRead(inLight);
      rain = digitalRead(inRain);

    } //-------------------------Fin Modulo 1

    Buttonslcd();
    // Leer M�dulo 2
    if (i == 1) {

      digitalWrite(Sen_SelectA, HIGH);
      digitalWrite(Sen_SelectB, LOW);
      delay(1100);

      temperature[1] = dht.readTemperature();
      humidity[1] = dht.readHumidity();
      ammonia[1] = analogRead(inAmmonia);
      fire[1] = digitalRead(inFire);


    } //-------------------------Fin Modulo 2
    Buttonslcd();
    // Leer M�dulo 3
    if (i == 2) {

      digitalWrite(Sen_SelectA, LOW);
      digitalWrite(Sen_SelectB, HIGH);
      delay(1100);

      temperature[2] = dht.readTemperature();
      humidity[2] = dht.readHumidity();
      ammonia[2] = analogRead(inAmmonia);
      fire[2] = digitalRead(inFire);


    }// Fin modulo 3

    Buttonslcd();
    // Leer M�dulo 4
    if (i == 3) {

      digitalWrite(Sen_SelectA, HIGH);
      digitalWrite(Sen_SelectB, HIGH);
      delay(1100);

      temperature[3] = dht.readTemperature();
      humidity[3] = dht.readHumidity();
      ammonia[3] = analogRead(inAmmonia);
      fire[3] = digitalRead(inFire);
      ;

    } //Fin modulo 4
    Buttonslcd();

  } // Fin For Modulos cou



  //Serial.println ("Fuego: ");
  //Serial.println (fire[0]);
  //Serial.println (fire[1]);
  //Serial.println (fire[2]);
  //Serial.println (fire[3]);

}//--------------------------------FIN VOID  Read Sensor---- ----------------------------FIN VOID  Read Sensor----------------------------FIN VOID  Read Sensor-----------

// ----------------------  VOID SET DATE TIME// ----------------------  VOID SET DATE TIME// ----------------------  VOID SET DATE TIME// ----------------------  VOID SET DATE TIME// ----------------------  VOID SET DATE TIME

void setDateTime() {

  byte second =      00; //0-59
  byte minute =      42; //0-59
  byte hour =        14; //0-23
  byte weekDay =     1; //1-7
  byte monthDay =    27; //1-31
  byte month =       11; //1-12
  byte year  =       16; //0-99

  Wire.beginTransmission(DS1307_ADDRESS);
  Wire.write(zero); //stop Oscillator

  Wire.write(decToBcd(second));
  Wire.write(decToBcd(minute));
  Wire.write(decToBcd(hour));
  Wire.write(decToBcd(weekDay));
  Wire.write(decToBcd(monthDay));
  Wire.write(decToBcd(month));
  Wire.write(decToBcd(year));

  Wire.write(zero); //start

  Wire.endTransmission();
}// ---------------------- FIN VOID SET DATE TIME// ---------------------- FIN VOID SET DATE TIME// ----------------------  VOID SET DATE TIME// ----------------------  VOID SET DATE TIME
//------------------------ VOID PRINT LCD ------------------------ VOID PRINT LCD ------------------------ VOID PRINT LCD ------------------------ VOID PRINT LCD ------------------------ VOID PRINT LCD

void PrintTimeLcd() {

  // Reset the register pointer
  Wire.beginTransmission(DS1307_ADDRESS);
  Wire.write(zero);
  Wire.endTransmission();

  Wire.requestFrom(DS1307_ADDRESS, 7);

  int second = bcdToDec(Wire.read());
  int minute = bcdToDec(Wire.read());
  int hour = bcdToDec(Wire.read() & 0b111111); //24 hour time
  int weekDay = bcdToDec(Wire.read()); //0-6 -> sunday - Saturday
  int monthDay = bcdToDec(Wire.read());
  int month = bcdToDec(Wire.read());
  int year = bcdToDec(Wire.read());

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(monthDay);
  lcd.setCursor(2, 0);
  lcd.print("/");
  lcd.setCursor(3, 0);
  lcd.print(month);
  lcd.setCursor(5, 0);
  lcd.print("/");
  lcd.setCursor(6, 0);
  lcd.print(year);

  if (manual_mode == true) {
    lcd.setCursor(7, 1);
    lcd.print("MM");
  }

  if (manual_mode == false) {
    lcd.setCursor(7, 1);
    lcd.print("MA");
  }


  lcd.setCursor(11, 0);
  lcd.print("CSA");

  lcd.setCursor(1, 1);
  lcd.print(hour);
  lcd.setCursor(3, 1);
  lcd.print(":");
  lcd.setCursor(4, 1);
  lcd.print(minute);

  lcd.setCursor(10, 1);
  lcd.print("SYSTEM");
  delay(250);

}
// ------------------------FIN  VOID PRINT LCD------------------------FIN  VOID PRINT LCD ------------------------FIN  VOID PRINT LCD ------------------------FIN  VOID PRINT LCD ------------------------FIN  VOID PRINT LCD

byte decToBcd(byte val) {
  // Convert normal decimal numbers to binary coded decimal
  return ( (val / 10 * 16) + (val % 10) );
}

byte bcdToDec(byte val)  {
  // Convert binary coded decimal to normal decimal numbers
  return ( (val / 16 * 10) + (val % 16) );
}

void Auto_Actuadores() {

  //---------- Encender luces a la hora --------

  Wire.beginTransmission(DS1307_ADDRESS);
  Wire.write(zero);
  Wire.endTransmission();

  Wire.requestFrom(DS1307_ADDRESS, 7);

  int second = bcdToDec(Wire.read());
  int minute = bcdToDec(Wire.read());
  int hour = bcdToDec(Wire.read() & 0b111111); //24 hour time
  int weekDay = bcdToDec(Wire.read()); //0-6 -> sunday - Saturday
  int monthDay = bcdToDec(Wire.read());
  int month = bcdToDec(Wire.read());
  int year = bcdToDec(Wire.read());

  //--------------------  LUCES ON OFF HORA---------------

  if (minute >= 15 && minute <= 17)
  {

    if (!Alarms[4]) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Hora Programada");
      lcd.setCursor(0, 1);
      lcd.print("Luces= ON");
      Sonido_Alerta();
      delay (2500);

      digitalWrite(luz_led, HIGH);
      digitalWrite(autoLight, HIGH);
      Alarms[4] = true;
      luz_ON = true;
      delay(2500);
    }

  } // Fin del if minutos

  // Corregir intervalos  OFF
  if (minute < 15 || minute > 17)
  {
    if (Alarms[4]) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Hora Programada");
      lcd.setCursor(0, 1);
      lcd.print("Luces= OFF");
      Sonido_Alerta();
      delay (2500);
      digitalWrite(autoLight, LOW);
      digitalWrite(luz_led, LOW);
      Alarms[4] = false;
      luz_ON = false;
      delay(500);
    }
  }
  //--------------------FIN  LUCES ON OFF HORA---------------

  //---------- FIN  Encender luces a la hora -------------

  //---------- Encender Abanicos temperatura alta--------

  if (temperature[0] > 34 || temperature[1] > 34 || temperature[2] > 34 || temperature[3] > 34)
  {
    if (!Alarms[0]) {

      for (int i = 0; i < modulesCount; ++i)  // Muestra qeu modulo tiene la temperatura alta
      {
        if (temperature[i] > 34 ) {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Temperatura Alta"); /// LCD
          lcd.setCursor(0, 1);
          lcd.print("Modulo: ");
          lcd.setCursor(7, 1);
          lcd.print(i + 1);
          Sonido_Alerta();
          delay(3500);

        } //Fin if
      } // Fin For

      digitalWrite(autoAbanico, HIGH);
      abanico_ON = true;
      Alarms[0] = true;
      delay(2000);
    }

  } // .----- FIN if temperatura alta ON

  if (temperature[0] > 0 && temperature[1] > 0 && temperature[2] > 0 && temperature[3] > 0) {


    if (temperature[0] < 34 && temperature[1] < 34 && temperature[2] < 34 && temperature[3] < 34) {

      if (Alarms[0]) {

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Temperatura");
        lcd.setCursor(0, 1);
        lcd.print("Controlada");
        Sonido_Alerta();
        delay (2000);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Apagando");
        lcd.setCursor(0, 1);
        lcd.print("Ventiladores...");
        delay (3000);
        digitalWrite(autoAbanico, LOW);
        Alarms[0] = false;
        abanico_ON = false;
        delay(2000);
      }

    }// .----- FIN if temperatura alta ON

    //----------FIN Encender Abanicos temperatura alta--------

  }// fin del if > 0C�

}
//-------------------------------------FIN VOID Auto Actuador--------------------------------------FIN VOID Auto Actuador--------------------------------------------------------FIN VOID Auto Actuador----


//-------------------------------------VOID BOTONES LCD---------------------VOID BOTONES LCD-------------------VOID BOTONES LCD------------------------------------

void Buttonslcd() {

  // Select :740 756 765  if(B>=725 && B<=755)
  // derecha : 0 33  if(B>= 0 && B<=50)
  // abajo: : 327    if(B>=320 && B<=335)
  //Arriba  : 142 150      if(B>=130 && B<=260)
  // izquierda: 502 503  if(B>=500 && B<=520)

  B = analogRead (inbuttonlcd);

  // ------------------------Izquierda presionado para MOSTRAR SENSORES
  if (B >= 500 && B <= 590) {

    while (B >= 500 && B <= 590) {

      Print_All_Sensor();

      B = analogRead (inbuttonlcd);

      if (B > 1000)
      {
        // Salir si B es Mayor que 1000
        break;
      }
      //--- fin if
    }
    // Fin While
  }
  // FIN IF ------------ FIN Izquierda presionado para MOSTRAR SENSORES------

  B = analogRead (inbuttonlcd);

  // SI SE PRECIONA BOTON DERECHA  MODO: MANUAL
  // if(B>= 0 && B<=50 && auto_mode == true) {


  while (B >= 0 && B <= 50 && auto_mode == true)   {
    // statement
    lcd.clear();
    lcd.setCursor(3, 0); //   (0-16 , 0-1)
    lcd.print("MODO MANUAL");
    lcd.setCursor(0, 1); //   (0-16 , 0-1)
    lcd.print("    ACTIVADO   ");
    delay(3500);

    B = analogRead (inbuttonlcd);

    if (B > 50) {

      auto_mode = false;
      manual_mode = true;
      break;
    }

  }

  if (manual_mode == true) {

    digitalWrite(led_m_a, HIGH); // Enciende led manual/auto

    digitalWrite(Act_SelectA, LOW);
    digitalWrite(Act_SelectB, LOW);
    // ARRIBA presionado para  ENCENDER LUCES
    B = analogRead (inbuttonlcd);

    if (B >= 130 && B <= 260 && luz_ON == false) {

      while (B >= 130 && B <= 260 ) {

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("LUCES ");
        delay(100);

        B = analogRead (inbuttonlcd);

        Luces_ON();
        Actuadores_final();

        if (B > 1000)
        {
          luz_ON = true;
          break;
        }
        //--- fin if
      }
      // Fin While
    }
    // FIN IF ------------ FIN // ARRIBA presionado para  ENCENDER LUCES   ------

    // ARRIBA presionado para  APAGAR LUCES
    B = analogRead (inbuttonlcd);

    if (B >= 130 && B <= 260 && luz_ON == true) {

      while (B >= 130 && B <= 260) {

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("LUCES ");
        delay(100);

        B = analogRead (inbuttonlcd);

        Luces_OFF();
        Actuadores_final();

        if (B > 1000)
        {

          luz_ON = false;
          break;
        }
        //--- fin if
      }
      // Fin While
    }
    // ---------------FIN IF ARRIBA presionado para  apagar LUCES

    // ---------------FIN ON OFF LUCES--------------FIN ON OFF LUCES--------------FIN ON OFF LUCES--------------FIN ON OFF LUCES

    //--------------------  abajo abanico ON -----------------------
    B = analogRead (inbuttonlcd);

    if (B >= 300 && B <= 400 && abanico_ON == false)
    {
      // abajo presionado para  ENCENDER abanicos

      while (B >= 300 && B <= 400) {

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("ABANICOS ");
        Abanico_ON();
        Actuadores_final();
        delay(100);
        B = analogRead (inbuttonlcd);

        if (B > 1000) {

          abanico_ON = true;
          break;
        }
        //--- fin if
      }
      // Fin While
    }
    // FIN IF ------------ FIN Abajo presionado: ON ABANICOS

    //--------------------  abajo abanico ON -----------------------

    B = analogRead (inbuttonlcd);

    // Abajo presionado para  apagar abaanicos
    if (B >= 300 && B <= 400 && abanico_ON == true)
    {
      while (B >= 300 && B <= 400) {

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("ABANICOS ");
        Abanico_OFF();
        Actuadores_final();

        B = analogRead (inbuttonlcd);

        if (B > 1000)
        {

          abanico_ON = false;
          break;
        }
        //--- fin if
      }
      // Fin While
    }
    //  FIN IF
  }
  // FIN IF MODO MANUAL ON

  B = analogRead (inbuttonlcd);

  // BOTON DERECHA
  //if (B>= 0 && B<=50 && manual_mode == true){

  while (B >= 0 && B <= 50 && manual_mode == true) {

    lcd.clear();
    lcd.setCursor(0, 0); //   (0-16 , 0-1)
    lcd.print("MODO AUTOMATICO");
    lcd.setCursor(0, 1); //   (0-16 , 0-1)
    lcd.print("    ACTIVADO   ");
    delay(3500);

    B = analogRead (inbuttonlcd);

    if (B > 50) {

      auto_mode = true;
      manual_mode = false;
      break;
    }


  }

  //-----------------------------AUTO MODE ON----------------------------
  PrintTimeLcd();

  if (auto_mode == true) {

    digitalWrite(led_m_a, LOW); // apaga led manual/auto

    digitalWrite(Act_SelectA, LOW);
    digitalWrite(Act_SelectB, HIGH);
    delay (800);

    //Auto_Actuadores(); // LUCES y Ventiladores por hora y temperatura.

    // METER VOID DE ACCIONES AUTOMATICAS
    B = analogRead (inbuttonlcd);
    // Abajo Abanicos desactivados
    if (B >= 300 && B <= 400) {
      lcd.clear();
      lcd.setCursor(0, 0); //   (0-16 , 0-1)
      lcd.print(" ABANICO MANUAL ");
      lcd.setCursor(0, 1); //   (0-16 , 0-1)
      lcd.print("  DESACTIVADO ");
      delay(3500);


    } ///  FIN IF  Manual desactivado

    B = analogRead (inbuttonlcd);



    // ARRIBA Modo manual ON-OFF Luces
    if (B >= 130 && B <= 260) {
      lcd.clear();
      lcd.setCursor(0, 0); //   (0-16 , 0-1)
      lcd.print(" LUCES MANUALES ");
      lcd.setCursor(0, 1); //   (0-16 , 0-1)
      lcd.print("  DESACTIVADAS  ");
      delay(3500);


    } //  FIN IF  Manual desactivado

  } // ---- FIN MODO  auto_mode==true


}//---------------------------FIN VOID BUTTONS LCD--------------------------------FIN VOID BUTTONS LCD-----------------FIN VOID BUTTONS LCD--------------------

//--------------------VOID ALERTAS--------------------VOID ALERTAS--------------------VOID ALERTAS--------------------VOID ALERTAS--------------------VOID ALERTAS

void Alertas() {


  lluvia(); //----------------ALERTA LLUVIA----------

  // PrintTimeLcd();
  //Buttonslcd();

  fuego(); //----------------ALERTA Fuego ----------

  // PrintTimeLcd();
  // Buttonslcd();


  humedad(); //------------Alerta humedad Alta----------

  //  PrintTimeLcd();
  // Buttonslcd();


  temperatura(); //------------Alerta Temperatura Alta----------

  // PrintTimeLcd();
  // Buttonslcd();

  amoniaco();  //--------------Alerta Calidad de Aire------
}
// ---------------------------FIN VOID ALERTAS-------------------------------------

//---------------------- VOID AIRE--------------------- VOID AIRE--------------------- VOID AIRE--------------------- VOID AIRE---------------------

void amoniaco() {

  for (int i = 0; i < modulesCount; ++i)
  {


    if (!Alarms[7]) { // If cuando no hay alertas

      if (ammonia[i] > 500) { // Pregunta si hay alertas de Aire contaminado

        digitalWrite (outStopAlarm, HIGH);
        delay(500);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Comprobando...");
        lcd.setCursor(0, 1);
        lcd.print("ALERTA AMONIACO");
        delay(2000);

        digitalWrite (outStopAlarm, HIGH);
        delay(500);
        digitalWrite (outStopAlarm, LOW);
        delay(500);
        digitalWrite (outStopAlarm, HIGH);
        delay(500);
        digitalWrite (outStopAlarm, LOW);


        ReadSensor_confir();


        if (ammonia[i] > 500) {

          digitalWrite (outStopAlarm, HIGH);
          delay(500);
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("AIRE CONTAMINADO");
          lcd.setCursor(0, 1);
          lcd.print("MODULO");
          lcd.setCursor(7, 1);
          lcd.print(i + 1);
          lcd.setCursor(9, 1);
          lcd.print(ammonia[i]);
          Sonido_Alerta();

          ammonia_confirmacion = true;

          //  Llamada_cel();
          // Mensaje_aire();
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("PRESIONE ALERTA");
          lcd.setCursor(0, 1);
          lcd.print("PARA CANCELAR");
          digitalWrite (outStopAlarm, HIGH);
          delay(1000);
          digitalWrite (outStopAlarm, LOW);
          delay(1000);
          digitalWrite (outStopAlarm, HIGH);

        } // Fin del IF (!Alarms[]

      } // if comprobacion

    } // Fin si hay alerta de incendio


  } // FIN del FOR

} //------------------------------FIN VOID AIRE-------------------FIN VOID AIRE-------------------FIN VOID AIRE-------------------FIN VOID AIRE


//-----------Void Temperatura   ---------Void Temperatura---------Void Temperatura---------Void Temperatura---------Void Temperatura---------Void Temperatura

void temperatura() {

  for (int i = 0; i < modulesCount; ++i)
  {


    if (!Alarms[2]) { // If cuando no hay alertas de tempratura


      if (temperature[i] >= 38) {

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Comprobando...");
        lcd.setCursor(0, 1);
        lcd.print("ALERTA TEMP/TURA");

        digitalWrite (outStopAlarm, HIGH);
        delay(500);
        digitalWrite (outStopAlarm, LOW);
        delay(500);
        digitalWrite (outStopAlarm, HIGH);
        delay(500);
        digitalWrite (outStopAlarm, LOW);

        ReadSensor_confir();

        if (temperature[i] >= 38) {


          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("ALTA TEMP/TURA");
          lcd.setCursor(0, 1);
          lcd.print("MODULO ");
          lcd.setCursor(7, 1);
          lcd.print(i + 1);
          lcd.setCursor(9, 1);
          lcd.print(temperature[i]);
          lcd.setCursor(11, 1);
          lcd.print("C*");
          Sonido_Alerta();


          temp_confirmacion = true;
          //Mensaje_temperatura();
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("ALTA TEMP/TURA");
          lcd.setCursor(0, 1);
          lcd.print("Llamando...");
          Llamada_cel();
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("PRESIONE ALERTA");
          lcd.setCursor(0, 1);
          lcd.print("PARA CANCELAR");
          digitalWrite (outStopAlarm, HIGH);
          delay(1000);
          digitalWrite (outStopAlarm, LOW);
          delay(1000);
          digitalWrite (outStopAlarm, HIGH);

        } // Fin del IF (!Alarms[]

      } // If confirmacion

    } // Fin si hay alerta de incendio


  } // FIN del FOR


}
//---------FIN Void Temperatura  //---------FIN Void Temperatura  //---------FIN Void Temperatura  //---------FIN Void Temperatura  //---------FIN Void Temperatura


//------------------VOID HUMEDAD----------------------VOID HUMEDAD----------------VOID HUMEDAD-----VOID HUMEDAD-----VOID HUMEDAD--

void humedad() {

  for (int i = 0; i < modulesCount; ++i)
  {

    if (!Alarms[1]) {


      if (humidity[i] > 70 && humidity[i] < 100) {

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Comprobando...");
        lcd.setCursor(0, 1);
        lcd.print("ALERTA HUMEDAD");

        digitalWrite (outStopAlarm, HIGH);
        delay(500);
        digitalWrite (outStopAlarm, LOW);
        delay(500);
        digitalWrite (outStopAlarm, HIGH);
        delay(500);
        digitalWrite (outStopAlarm, LOW);

        ReadSensor_confir();


        if (humidity[i] > 70 && humidity[i] < 100) {


          // Si detecta una alerta debe asegurarse de que sea real. revison del parametro


          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("ALTA HUMEDAD");
          lcd.setCursor(0, 1);
          lcd.print("MODULO: ");
          lcd.setCursor(8, 1);
          lcd.print(i + 1);
          lcd.setCursor(10, 1);
          lcd.print("H: ");
          lcd.setCursor(13, 1);
          lcd.print(humidity[i]);
          lcd.setCursor(15, 1);
          lcd.print("%");
          Sonido_Alerta();



          // Llamada_cel();
          // Mensaje_humedad();
          humedad_confirmacion = true;
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("PRESIONE ALERTA");
          lcd.setCursor(0, 1);
          lcd.print("PARA CANCELAR");
          digitalWrite (outStopAlarm, HIGH);
          delay(1000);
          digitalWrite (outStopAlarm, LOW);
          delay(1000);
          digitalWrite (outStopAlarm, HIGH);



        } // Fin del IF (!Alarms[]

      } // Fin del If comprobacion

    } // Fin si hay alerta de humedaD


  } // Fin del for modulesCount


}
//----------------FIN VOID HUMEDAD--------------------FIN VOID HUMEDAD-------------------------FIN VOID HUMEDAD--------------


//--------------------------------Void FUEGO------------------------Void FUEGO------------------------Void FUEGO------------------------Void FUEGO------------------------Void FUEGO
void fuego() {


  int luz = analogRead (inLight);


  for (int i = 0; i < modulesCount; ++i) {

    if (!Alarms[3])
    {


      luz = analogRead (inLight);
      //-----------------------FUEGO OBTION BY ALARMS
      if (fire[i] == LOW && luz < 100) {

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Comprobando...");
        lcd.setCursor(2, 1);
        lcd.print("ALERTA FUEGO");


        digitalWrite (outStopAlarm, HIGH);
        delay(500);
        digitalWrite (outStopAlarm, LOW);
        delay(500);
        digitalWrite (outStopAlarm, HIGH);
        delay(500);
        digitalWrite (outStopAlarm, LOW);

        ReadSensor_confir();

        if (fire[i] == LOW) {



          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("ALERTA INCENDIO");
          lcd.setCursor(0, 1);
          lcd.print("MODULO: ");
          lcd.setCursor(8, 1);
          lcd.print(i + 1);
          lcd.setCursor(10, 1);
          lcd.print("F: ");
          lcd.setCursor(13, 1);
          lcd.print(fire[i]);
          Sonido_Alerta();

          //Mensaje_fuego();
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("INCENDIO!!!");
          lcd.setCursor(0, 1);
          lcd.print("Llamando...");
          //Llamada_cel();
          fuego_confirmacion = true;
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("PRESIONE ALERTA");
          lcd.setCursor(0, 1);
          lcd.print("PARA CANCELAR");
          digitalWrite (outStopAlarm, HIGH);
          delay(1000);
          digitalWrite (outStopAlarm, LOW);
          delay(1000);
          digitalWrite (outStopAlarm, HIGH);




        } //  Fin si hay alerta de incendio

      } // if comprobacion
    } // Fin del IF (!Alarms[3]
  } // FIN del FOR


} //------------------FIN VOID FUEGO//------------------FIN VOID FUEGO//------------------FIN VOID FUEGO//------------------FIN VOID FUEGO//------------------FIN VOID FUEGO//------------------FIN VOID FUEGO



//-------------LLUVIA-----------LLUVIA-----------LLUVIA---------//-------------LLUVIA-----------LLUVIA-----------LLUVIA---------//-------------LLUVIA-----------LLUVIA-----------LLUVIA---------

void lluvia() {


  rain = digitalRead(inRain);



  if (!Alarms[5]) { // ------If que pregunta si esta lloviendo para generar alerta


    rain = digitalRead(inRain);

    if (rain == LOW) { // Esta lloviendo?

      digitalWrite (outStopAlarm, HIGH);
      delay(500);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Comprobando...");
      lcd.setCursor(2, 1);
      lcd.print("ALERTA LLUVIA");

      digitalWrite (outStopAlarm, HIGH);
      delay(500);
      digitalWrite (outStopAlarm, LOW);
      delay(500);
      digitalWrite (outStopAlarm, HIGH);
      delay(500);
      digitalWrite (outStopAlarm, LOW);

      rain = digitalRead(inRain);

      if (rain == LOW) { // Confirmada la lluvia

        /* code */
        digitalWrite (outStopAlarm, HIGH);
        delay(500);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("ESTA LLOVIENDO!");
        lcd.setCursor(0, 1);
        lcd.print("LLAMANDO...");
        Sonido_Alerta();
        delay(2500);
        digitalWrite (outStopAlarm, LOW);
        delay(500);
        digitalWrite (outStopAlarm, HIGH);
        delay(500);

        // Mensaje_lluvia();
        //  Llamada_cel();
        lluvia_confirmacion = true;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("PRESIONE ALERTA");
        lcd.setCursor(0, 1);
        lcd.print("PARA CANCELAR");
        digitalWrite (outStopAlarm, HIGH);
        delay(1500);
        digitalWrite (outStopAlarm, LOW);
        delay(1500);
        digitalWrite (outStopAlarm, HIGH);


      } /// ------FIN IF Lluvia confirmada

    } // Fin if comprobacion

  }//-- FIN IF pregunta? SI ESTA LLOVIENDO


}//-------------FIN Void LLUVIA----------- Void FIN LLUVIA-----------FIN Void LLUVIA---------------------FIN Void LLUVIA----------- Void FIN LLUVIA-----------FIN Void LLUVIA---------

//---------------------------VOID CONFIRMACION ALERTA---------------------------------------------------------
void confir_alerta () {

  boolean alert_des = false;

  // ENTRA DEPENDIENDO DEL ESTADO DE LA ALRMA DE CADA VARIABLE Y BOTON

  //----------FIN  if QUE APAGA EL led

  B = analogRead (inbuttonlcd);
  //------------------------------Lluvia----------------

  if ((B >= 725 && B <= 780) && lluvia_confirmacion == true) {


    digitalWrite (outStopAlarm, HIGH);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ALERTA LLUVIA");
    lcd.setCursor(0, 1);
    lcd.print("RECIBIDA");
    delay(2000);
    Alarms[5] = true; // alerta fuego

  } // fin if boton confirmacion


  if (lluvia_confirmacion == true) {


    rain = digitalRead(inRain);

    if (rain == HIGH) {

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("ALERTA LLUVIA");
      lcd.setCursor(0, 1);
      lcd.print("CONTROLADA");
      delay(2000);
      digitalWrite (outStopAlarm, LOW);
      lluvia_confirmacion = false;
      Alarms[5] = false; // alerta lluvia
      alert_des = true;
    } // fin if boolean lluvia
  }// fin if lluvia confirmacion

  //------------------------------FIn Lluvia----------------
  B = analogRead (inbuttonlcd);
  //------------------------------FUEGO ----------------

  if ((B >= 725 && B <= 780) && fuego_confirmacion == true) {

    digitalWrite (outStopAlarm, HIGH);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ALERTA FUEGO");
    lcd.setCursor(0, 1);
    lcd.print("RECIBIDA");
    delay(2000);
    Alarms[3] = true; // alerta fuego

  } // fin if boton confirmacion fuego

  if (fuego_confirmacion == true) {

    ReadSensor_confir();

    if (fire[0] == HIGH && fire[1] == HIGH && fire[2] == HIGH && fire[3] == HIGH) {

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("ALERTA FUEGO");
      lcd.setCursor(0, 1);
      lcd.print("CONTROLADA");
      delay(2000);
      digitalWrite (outStopAlarm, LOW);
      fuego_confirmacion = false;
      Alarms[3] = false; // alerta FUEGO
      alert_des = true;
    } // fin if boolean fuego

  } // final IF

  //------------------------------FIN FUEGO ----------------
  B = analogRead (inbuttonlcd);
  //------------------------------TEMPERATURA ----------------

  // PARA QUE NO LLAME MAS SE CAMBIA EL BOOLEANO ALERTA [I] A TRUE "DEPENDIENDO DE LA ALERTA"

  if ((B >= 725 && B <= 780) && temp_confirmacion == true) {

    digitalWrite (outStopAlarm, HIGH);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ALERTA TEMP/TURA");
    lcd.setCursor(0, 1);
    lcd.print("RECIBIDA");
    delay(2000);
    Alarms[2] = true; // alerta fuego

  } // fin if boton confirmacion fuego

  if (temp_confirmacion == true) {



    ReadSensor_confir();


    if (temperature[0] < 40  && temperature[1] < 40  && temperature[2] < 40  && temperature[3] < 40 && temperature[0] > 0  && temperature[1] > 0 > 0  && temperature[2] > 0  && temperature[3] > 0) {

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("ALERTA TEMP/TURA");
      lcd.setCursor(0, 1);
      lcd.print("CONTROLADA");
      delay(2000);
      digitalWrite (outStopAlarm, LOW);
      temp_confirmacion = false;
      Alarms[2] = false; // alerta FUEGO
      alert_des = true;

    } // fin if boolean fuego
  }

  //------------------------------fin TEMPERATURA ----------------
  B = analogRead (inbuttonlcd);
  //------------------------------ AMONIA ----------------

  if ((B >= 725 && B <= 780) && ammonia_confirmacion == true) {

    digitalWrite (outStopAlarm, HIGH);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ALERTA AMONIACO");
    lcd.setCursor(0, 1);
    lcd.print("RECIBIDA");
    delay(2000);
    Alarms[7] = true; // alerta fuego

  } // fin if boton confirmacion fuego

  if (ammonia_confirmacion == true) {



    ReadSensor_confir();


    if (ammonia[0] < 800 && ammonia[1] < 800 && ammonia[2] < 800 && ammonia[3] < 800
        && ammonia[0] > 0 && ammonia[1] > 0 && ammonia[2] > 0 && ammonia[3] > 0) {

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("AIRE");
      lcd.setCursor(0, 1);
      lcd.print("RANGO TOLERABLE");
      delay(2000);
      digitalWrite (outStopAlarm, LOW);
      ammonia_confirmacion = false;
      Alarms[7] = false;
      alert_des = true;
    }

  }
  //------------------------------ FIN AMONIA ----------------
  B = analogRead (inbuttonlcd);
  //---------------------------------HUMEDAD--------------------
  if ((B >= 725 && B <= 780) && humedad_confirmacion == true) {

    digitalWrite (outStopAlarm, HIGH);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ALERTA HUMEDAD");
    lcd.setCursor(0, 1);
    lcd.print("RECIBIDA");
    delay(2000);
    Alarms[1] = true; // alerta fuego


  } // fin if boton confirmacion fuego

  if (humedad_confirmacion == true) {

    ReadSensor_confir();

    if (humidity[0] < 70 && humidity[1] < 70 && humidity[2] < 70 && humidity[3] < 70 && humidity[0] > 0
        && humidity[1] > 0 && humidity[2] > 0 && humidity[3] > 0) {

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("ALERTA HUMEDAD");
      lcd.setCursor(0, 1);
      lcd.print("CONTROLADA");
      delay(2000);
      digitalWrite (outStopAlarm, LOW);
      humedad_confirmacion = false;
      Alarms[1] = false; // alerta FUEGO
      alert_des = true;

    } // fin if boolean fuego

  } // final IF

  //-------------------------------FIN HUMEDAD--------------------
  //---------- if QUE APAGA EL led

  if (rain == HIGH && fire[0] == LOW && fire[1] == LOW && fire[2] == LOW && fire[3] == LOW
      && temperature[0] < 40  && temperature[1] < 40  && temperature[2] < 40  && temperature[3] < 40
      && temperature[0] > 0  && temperature[1] > 0 > 0  && temperature[2] > 0  && temperature[3] > 0
      && ammonia[0] < 800 && ammonia[1] < 800 && ammonia[2] < 800 && ammonia[3] < 800
      && ammonia[0] > 0 && ammonia[1] > 0 && ammonia[2] > 0 && ammonia[3] > 0 && humidity[0] < 70
      && humidity[1] < 70 && humidity[2] < 70 && humidity[3] < 70 && humidity[0] > 0 && humidity[1] > 0
      && humidity[2] > 0 && humidity[3] > 0 && alert_des == true) {


    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ALERTAS");
    lcd.setCursor(0, 1);
    lcd.print("DESACTIVADAS");
    delay(2000);
    digitalWrite (outStopAlarm, LOW);
    alert_des = false;


  } // FIN IF APAGA LED ALERTA!
} // FIN VOID COMFIRMACION ALERTA!!!
//------------------FIN VOID CONFIRMACION ALERTA-------------------------------------------------

//-----------------------------VOID PRINT ALL SENSORS----------------------------------------------

void Print_All_Sensor() {

  lcd.clear();
  lcd.setCursor(0, 0); //   (0-16 , 0-1)
  lcd.print("VALORES SENSORES");
  delay(2000);
  //----------------------------- case 1: S1
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SENSOR 1:");

  //--------------------------S1
  lcd.setCursor(2, 1);
  lcd.print("T:");
  lcd.setCursor(4, 1);
  lcd.print(temperature[0]);
  lcd.print("C");

  lcd.setCursor(8, 1);
  lcd.print("H:");
  lcd.setCursor(10, 1);
  lcd.print(humidity[0]);
  lcd.print("%");

  lcd.setCursor(11, 0);
  lcd.print("A:");
  lcd.setCursor(13, 0);
  lcd.print(ammonia[0]);

  delay(3000);
  //----------------------------- case 2: S2
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SENSOR 2:");

  //------------------------- S2
  lcd.setCursor(2, 1);
  lcd.print("T:");
  lcd.setCursor(4, 1);
  lcd.print(temperature[1]);
  lcd.print("C");

  lcd.setCursor(8, 1);
  lcd.print("H:");
  lcd.setCursor(10, 1);
  lcd.print(humidity[1]);
  lcd.print("%");

  lcd.setCursor(11, 0);
  lcd.print("A:");
  lcd.setCursor(13, 0);
  lcd.print(ammonia[1]);

  delay(3000);

  //----------------------------- case 3: S3
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SENSOR 3:");

  //--------------------------S3
  lcd.setCursor(2, 1);
  lcd.print("T:");
  lcd.setCursor(4, 1);
  lcd.print(temperature[2]);
  lcd.print("C");

  lcd.setCursor(8, 1);
  lcd.print("H:");
  lcd.setCursor(10, 1);
  lcd.print(humidity[2]);
  lcd.print("%");

  lcd.setCursor(11, 0);
  lcd.print("A:");
  lcd.setCursor(13, 0);
  lcd.print(ammonia[2]);

  delay (3000);
  //----------------------------- case 4: S4

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SENSOR 4:");

  //--------------------------S4
  lcd.setCursor(2, 1);
  lcd.print("T:");
  lcd.setCursor(4, 1);
  lcd.print(temperature[3]);
  lcd.print("C");

  lcd.setCursor(8, 1);
  lcd.print("H:");
  lcd.setCursor(10, 1);
  lcd.print(humidity[3]);
  lcd.print("%");

  lcd.setCursor(11, 0);
  lcd.print("A:");
  lcd.setCursor(13, 0);
  lcd.print(ammonia[3]);
  delay (3000);

  //________________________Caso 5: Imprime Los Valores

}//--------------------------FIN VOID print all sensors--------------------------------FIN VOID print all sensors---------------------------------FIN VOID print all sensors     -

//------------------------ VOID ACTUADORES FINAL-----------------------

void Actuadores_final() {

  int  A_final = digitalRead (inAbanico);
  int  L_final = digitalRead (inLuz);



  if (A_final == HIGH) {
    // // Secuencia de inicio
    digitalWrite (aba_led, HIGH);

    if (abanico_2_3_ON == false) {

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Encendiendo");
      lcd.setCursor(0, 1);
      lcd.print("Ventiladores...");
      delay (2000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("  ENCENDIENDO  ");
      lcd.setCursor(0, 1);
      lcd.print("ABANICO: ");
      lcd.setCursor(9, 1);
      lcd.print("1");
      digitalWrite(outAbanico, LOW);
      delay(3000);
      lcd.setCursor(9, 1);
      lcd.print("2");
      digitalWrite(outAbanico2, LOW);
      delay(3000);
      lcd.setCursor(9, 1);
      lcd.print("3");
      digitalWrite(outAbanico3, LOW);
      abanico_2_3_ON  = true;
      delay(1000);
    }

    if (abanico_2_3_ON  == true)
    {
      //__ Actuadores-----
      digitalWrite(outAbanico, LOW);
      digitalWrite(outAbanico2, LOW);
      digitalWrite(outAbanico3, LOW);
    }
  }

  if (A_final == LOW) {
    //__ Actuadores-----
    digitalWrite (aba_led, LOW);
    digitalWrite(outAbanico, HIGH);
    digitalWrite(outAbanico2, HIGH); //
    digitalWrite(outAbanico3, HIGH); //
    abanico_2_3_ON  = false;
  }

  if (L_final == HIGH) {

    digitalWrite (luz_led, HIGH);
    digitalWrite(outLight, LOW);
  }
  else {

    digitalWrite (luz_led, LOW);
    digitalWrite(outLight, HIGH);

  } // FIN ELSE-----
}
//-------------------------Fin Void Abanico-OFF----------------

//-------------------------Inicio Void Luces_ON----------------

void Luces_ON() {

  digitalWrite(manualLight, HIGH);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("LUCES = ON");
  delay(2000);

}
//-------------------------Inicio Void Luces_OFF----------------
void Luces_OFF() {
  digitalWrite(manualLight, LOW);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("LUCES = OFF");
  delay(2000);
}
//-------------------------Fin Void Luces_ON-OFF----------------

//-------------------------Inicio Void abanico_ON----------------
void Abanico_ON() {

  digitalWrite(manualAbanico, HIGH);
  abanico_ON = true;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Ventiladores= ON");
  delay(2000);

}

//-------------------------Inicio Void Abanico_OFF----------------
void Abanico_OFF() {

  digitalWrite(manualAbanico, LOW);
  abanico_ON = false;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Ventiladores=OFF");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("APAGANDO...");
  lcd.setCursor(0, 1);
  lcd.print("VENTILADORES");
  delay(2000);

}
//-------------------------Fin Void Abanico-OFF----------------

//-------------------------------------------VOID Show Serial Data----------------------VOID Show Serial Data----------------------VOID Show Serial Data----------------------VOID Show Serial Data

void ShowSerialData()
{
  while (serialGPRS.available() != 0) {
    Serial.write(serialGPRS.read());
  }
}//-------------------------------------------FIN VOID Show Serial Data----------------------V



//-----------------VOID POWER UP OR DOWN----------------------------------------------------
void powerUpOrDown()
{

  digitalWrite(outControlGprs, HIGH);
  delay(3000);
}
//-----------------FIN VOID POWER UP OR DOWN----------------------------------------------------

//---------------------------VOID SONIDO ALERTA //--------------------------- VOID SONIDO ALERTA //---------------------------VOID SONIDO ALERTA //--------------------------- VOID SONIDO ALERTA

void Sonido_Alerta() {

  tone (buzzer, 3000);
  delay(500);
  noTone (buzzer);
  delay(500);
  tone (buzzer, 3000);
  delay(500);
  noTone (buzzer);
  delay(500);
  tone (buzzer, 3000);
  delay(500);
  noTone (buzzer);
}
//--------------------------- FIN VOID SONIDO ALERTA //--------------------------- FIN VOID SONIDO ALERTA //--------------------------- FIN VOID SONIDO ALERTA //--------------------------- FIN VOID SONIDO ALERTA


//--------------------VOID FALLOS SENSORES------------------------------------------------
void fallos () {

  if (fallo == false) {


    for (int i = 0 ; i < modulesCount ; i++) {

      if (temperature[i] < 5) {

        delay (1000);

        ReadSensor_confir(); //confirmacion lectura

        if (temperature[i] < 5) {

          digitalWrite(led_fallos, HIGH);
          lcd.clear();
          lcd.setCursor(2, 0);
          lcd.print("FALLO SENSOR");
          lcd.setCursor(0, 1);
          lcd.print("TEMP MODULO: ");
          lcd.print(i + 1);
          fallo = true;
          delay (4000);
        }
      }
    }
  }//------------- Fin If Fallos==False

  if (fallo == true) {

    if (temperature[0] > 10 && temperature[1] > 10 && temperature[2] > 10 && temperature[3] > 10)
    {

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("SENSORES BIEN");
      lcd.setCursor(0, 1);
      lcd.print("CONECTADOS");
      fallo = false;
      delay (4000);

      digitalWrite(led_fallos, LOW);

    }

  } // FIN FOR
}
// ------------------------------FIN VOID FALLOS SENSORES --------------------------------------

//----------------------VOID READ SENSOR CONFIRMACION/VOID FALLOS-----------------------------
void ReadSensor_confir () {

  //Read one module per second

  for (int i = 0; i < modulesCount; i++) {

    if (i == 0) {

      digitalWrite(Sen_SelectA, LOW);
      digitalWrite(Sen_SelectB, LOW);
      delay(1200);

      temperature[0] = dht.readTemperature();
      humidity[0] = dht.readHumidity();
      ammonia[0] = analogRead(inAmmonia);
      fire[0] = digitalRead(inFire);
      light = analogRead(inLight);
      rain = digitalRead(inRain);
      delay(500);
    } //-------------------------Fin Modulo 1

    // Leer M�dulo 2
    if (i == 1) {

      digitalWrite(Sen_SelectA, HIGH);
      digitalWrite(Sen_SelectB, LOW);
      delay(1200);

      temperature[1] = dht.readTemperature();
      humidity[1] = dht.readHumidity();
      ammonia[1] = analogRead(inAmmonia);
      fire[1] = digitalRead(inFire);
      delay(500);

    } //-------------------------Fin Modulo 2

    // Leer M�dulo 3
    if (i == 2) {

      digitalWrite(Sen_SelectA, LOW);
      digitalWrite(Sen_SelectB, HIGH);
      delay(1200);

      temperature[2] = dht.readTemperature();
      humidity[2] = dht.readHumidity();
      ammonia[2] = analogRead(inAmmonia);
      fire[2] = digitalRead(inFire);
      delay (500);

    }// Fin modulo 3

    // Leer M�dulo 4
    if (i == 3) {

      digitalWrite(Sen_SelectA, HIGH);
      digitalWrite(Sen_SelectB, HIGH);
      delay(1200);

      temperature[3] = dht.readTemperature();
      humidity[3] = dht.readHumidity();
      ammonia[3] = analogRead(inAmmonia);
      fire[3] = digitalRead(inFire);
      delay (500);

    } //Fin modulo 4

  } // Fin For Modulos count


  //Serial.println ("Fuego: ");
  //Serial.println (fire[0]);
  //Serial.println (fire[1]);
  //Serial.println (fire[2]);
  //Serial.println (fire[3]);

}
//-------------------FIN VOID READ SENSOR CONFIRMACION/VOID FALLOS---------------------

//------------------------------INICIO VOIDS MENSAJES GSM--------------------------------------
void Mensaje_fuego() {

  ShowSerialData();
  serialGPRS.print("AT+CMGF=1\r");    //Because we want to send the SMS in text mode
  delay(100);
  serialGPRS.println("AT + CMGS = \"+573186912935\"");//send sms message, be careful need to add a country code before the cellphone number
  delay(100);
  serialGPRS.println("CSA SYSTEM: ALERTA FUEGO DETECTADO!!");//the content of the message
  delay(100);
  serialGPRS.println((char)26);//the ASCII code of the ctrl+z is 26
  delay(100);
  serialGPRS.println();
}

void Mensaje_lluvia() {

  ShowSerialData();
  serialGPRS.print("AT+CMGF=1\r");    //Because we want to send the SMS in text mode
  delay(100);
  serialGPRS.println("AT + CMGS = \"+573186912935\"");//send sms message, be careful need to add a country code before the cellphone number
  delay(100);
  serialGPRS.println("CSA SYSTEM: ALERTA LLUVIA DETECTADA!!");//the content of the message
  delay(100);
  serialGPRS.println((char)26);//the ASCII code of the ctrl+z is 26
  delay(100);
  serialGPRS.println();
}

void Mensaje_temperatura() {

  ShowSerialData();
  serialGPRS.print("AT+CMGF=1\r");    //Because we want to send the SMS in text mode
  delay(100);
  serialGPRS.println("AT + CMGS = \"+573186912935\"");//send sms message, be careful need to add a country code before the cellphone number
  delay(100);
  serialGPRS.println("CSA SYSTEM: ALERTA TEPERATURA SUPERIOR A LOS 45�C");//the content of the message
  delay(100);
  serialGPRS.println((char)26);//the ASCII code of the ctrl+z is 26
  delay(100);
  serialGPRS.println();
}

void Mensaje_aire() {

  ShowSerialData();
  serialGPRS.print("AT+CMGF=1\r");    //Because we want to send the SMS in text mode
  delay(100);
  serialGPRS.println("AT + CMGS = \"+573186912935\"");//send sms message, be careful need to add a country code before the cellphone number
  delay(100);
  serialGPRS.println("CSA SYSTEM: ALERTA CALIDAD DE AIRE CONTAMINADO");//the content of the message
  delay(100);
  serialGPRS.println((char)26);//the ASCII code of the ctrl+z is 26
  delay(100);
  serialGPRS.println();
}
void Mensaje_humedad() {

  ShowSerialData();
  serialGPRS.print("AT+CMGF=1\r");    //Because we want to send the SMS in text mode
  delay(100);
  serialGPRS.println("AT + CMGS = \"+573186912935\"");//send sms message, be careful need to add a country code before the cellphone number
  delay(100);
  serialGPRS.println("CSA SYSTEM: ALERTA HUMEDAD ALTA!");//the content of the message
  delay(100);
  serialGPRS.println((char)26);//the ASCII code of the ctrl+z is 26
  delay(100);
  serialGPRS.println();
}


//------------------------------FIN INICIO VOIDS MENSAJES GSM--------------------------------------

//---------------------------VOID LLAMADA CELULAR------------------------------------------------
void Llamada_cel() {

  String number = "3186912935";

  serialGPRS.println("AT+CREG?");
  delay(3000);
  ShowSerialData();
  delay(3000);
  String number2call = "ATD" + number + ";";
  serialGPRS.println(number2call);// Call
  delay(25000);
  ShowSerialData();
  serialGPRS.println("ATH");// Call End
  delay(3000);
  ShowSerialData();

}
//---------------------------FIN VOID LLAMADA CELULAR------------------------------------------------

//-----------------------------------VOID ENVIO DE DATOS-----------------------------------------
/*
  void SubmitHttpRequest()
  {
  String dataTemperature ="[";
  String dataHumidity = "[";
  String dataAmmonia = "[";
  String dataFire = "[";
  String dataLight =  String(light);
  String dataRain = String(rain);
  serialGPRS.println("AT+HTTPINIT"); //init the HTTP request

  delay(2000);
  ShowSerialData();

  for (int i = 0; i < modulesCount; i++)
  {
    //Code
    if(i == modulesCount-1){
      dataTemperature = dataTemperature + String(temporaltemperature[i]) + "]";
      dataHumidity = dataHumidity + String(temporalhumidity[i]) + "]";
      dataAmmonia = dataAmmonia + String(temporalammonia[i]) + "]";
      dataFire = dataFire + String(fire[i]) + "]";

      temporaltemperature[i] = 0;
      temporalhumidity[i] = 0;
      temporalammonia[i] = 0;

      //Serial.println("dataTemperature " + dataTemperature + " dataHumidity " + dataHumidity + " dataAmmonia " + dataAmmonia  );

      }else{
        dataTemperature = dataTemperature + String(temporaltemperature[i]) + ",";
        dataHumidity = dataHumidity + String(temporalhumidity[i]) + ",";
        dataAmmonia = dataAmmonia + String(temporalammonia[i])  + ",";
        dataFire = dataFire + String(fire[i])  + ",";


        temporaltemperature[i] = 0;
        temporalhumidity[i] = 0;
        temporalammonia[i] = 0;
      }


    }


    String data = "AT+HTTPPARA=\"URL\",\"http://190.146.201.43/Sgaa/public/measurement/mesurementGPRS?temperature="+dataTemperature+"&humidity="+dataHumidity+"&ammonia="+dataAmmonia+"&fire="+dataFire+"&light="+dataLight+"&rain="+dataRain+"&user_id=2\"";
  serialGPRS.println(data);// setting the httppara, the second parameter is the website you want to access
  delay(1000);

  ShowSerialData();
  serialGPRS.println("AT+HTTPACTION=0");//submit the request
  delay(10000);//the delay is very important, the delay time is base on the return from the website, if the return datas are very large, the time required longer.

  ShowSerialData();
  serialGPRS.println("AT+HTTPREAD");// read the data from the website you access
  delay(1000);
  ShowSerialData();
  delay(3000);
  serialGPRS.println("");
  delay(1000);

  }
*/
//---------------------------------FIN VOID ENVIO DE DATOS-----------------------------------------
