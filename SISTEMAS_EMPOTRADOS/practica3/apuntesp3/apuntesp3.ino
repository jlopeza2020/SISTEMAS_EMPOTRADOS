#include <TimerOne.h>

//#include <StaticThreadController.h>
//#include <Thread.h>
//#include <ThreadController.h>

/*#include <DHT.h>
#include <DHT_U.h>*/

#include <LiquidCrystal.h>
#include "LedThread.h"

#define DHTTYPE DHT11   // DHT 11

#define DHT11_PIN 13
#define LED_PIN1 10// red
#define LED_PIN2 5 // green 
#define TRIGGER_PIN 8
#define ECHO_PIN 7
int X;				// variable para almacenar valor leido del eje X
int Y;				// variable para almacenar valor leido del eje y
#define PULSADOR 9		// pulsador incorporado pin digital 9
int SW;				// variable para almacenar valor leido del pulsador
float distancia;

//lcd structure
#define BOTON 1;
#define RS 12
#define ENABLE 11
#define D0 6 
#define D1 4
#define D2 3 
#define D3 2
#define LCD_COLS 16
#define LCD_ROWS 2

LiquidCrystal lcd(RS, ENABLE, D0, D1, D2, D3);
//DHT dht(DHT11_PIN, DHTTYPE);
ThreadController controller = ThreadController();

int ledstate = LOW;

// define ISR
void blinkLED(){
  digitalWrite(LED_PIN1, ledstate);
  ledstate != ledstate;  
}
void setup() {
  // put your setup code here, to run once:

  //interruptions 
  Timer1.initialize(500000);
  Timer1.attachInterrupt(blinkLED);

  Serial.begin(9600);

  // First object LedThread with interval of 0.5 seconds
  ////LedThread* ledThread = new LedThread(LED_PIN1);
  ////ledThread->setInterval(500);
  ////controller.add(ledThread); // add thread to the controller

  
  
  //pinMode(LED_PIN1, OUTPUT);
  //pinMode(LED_PIN2, OUTPUT);
  //pinMode(TRIGGER_PIN, OUTPUT);
  //pinMode(ECHO_PIN, INPUT);
  //pinMode(PULSADOR, INPUT);		// pulsador como entrada

  //set up the LCD's number of columns and rows
  lcd.begin(LCD_COLS,LCD_ROWS);
  // Print a message to the LCD 
  lcd.print("   CARGANDO ...");
  

  //lcd.print("   Servicio");
  //delay(1000);


  //dht.begin();

}


void loop() {

  //if (counter < 3){
  controller.run(); // call controller each iteration
  //Serial.println(counter);
  //counter++;
  //}
  //else{
  //remove(ledThread) //- Removes the thread from the controller
  //}

  


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


  // Wait a few seconds between measurements.
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

}
