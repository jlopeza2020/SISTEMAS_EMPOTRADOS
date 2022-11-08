#include <DHT.h>
#include <DHT_U.h>



#include <LiquidCrystal.h>

#define DHTTYPE DHT11   // DHT 11

int DHT11_PIN = 13;
int LED_PIN1 = 10;// red
int LED_PIN2 = 5; // green 
int TRIGGER_PIN = 8;
int ECHO_PIN = 7;
int X;				// variable para almacenar valor leido del eje X
int Y;				// variable para almacenar valor leido del eje y
int PULSADOR = 9;		// pulsador incorporado pin digital 10
int SW;				// variable para almacenar valor leido del pulsador
float distancia;

LiquidCrystal lcd(12, 11, 6, 4, 3, 2);
DHT dht(DHT11_PIN, DHTTYPE);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  //pinMode(LED_PIN1, OUTPUT);
  //pinMode(LED_PIN2, OUTPUT);
  //pinMode(TRIGGER_PIN, OUTPUT);
  //pinMode(ECHO_PIN, INPUT);
  //pinMode(PULSADOR, INPUT);		// pulsador como entrada

  //set up the LCD's number of columns and rows
  //lcd.begin(16,2);
  // Print a message to the LCD 
  //lcd.print("hello, world! my love");
  //delay(1000);


  //dht.begin();



}


void loop() {
  //X = analogRead(A0);			// lectura de valor de eje x
  //Y = analogRead(A1);			// lectura de valor de eje y
  //SW = digitalRead(PULSADOR);		// lectura de valor de pulsador

  //digitalWrite(TRIGGER_PIN, HIGH);
  //delayMicroseconds(10);
  //digitalWrite(TRIGGER_PIN, LOW);
  //distancia=pulseIn(ECHO_PIN, HIGH);
  //Serial.println(distancia);
  //Serial.println(X);
  //Serial.println(Y);
  //Serial.println(SW);
  

  //analogWrite(LED_PIN2, 50);
  //delay(1000);
  //scroll 13 positions (string legth) to the left
  // to move it ofscreen left
  
  /*for( int positionCounter = 0; positionCounter < 13; positionCounter++){
    lcd.scrollDisplayLeft();

    //wait a bit
    delay(150);
  }*/


  /*// Wait a few seconds between measurements.
  delay(2000);

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  Serial.print(F(" Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("C "));
  Serial.print(f);
  Serial.print(F("F  Heat index: "));
  Serial.print(hic);
  Serial.print(F("C "));
  Serial.print(hif);
  Serial.println(F("F"));

  lcd.setCursor(0,0); 
  lcd.print("Temp: ");
  lcd.print(t);
  lcd.print((char)223);
  lcd.print("C");
  lcd.setCursor(0,1);
  lcd.print("Humidity: ");
  lcd.print(h);
  lcd.print("%");
  delay(1000);*/

  int sensorValue = analogRead(A4);
  // convert Analog2digital
  float voltage = sensorValue * (5.0 / 1023.0);
  Serial.println(voltage);
}


