#include <DHT.h>
#include <DHT_U.h>

#include <TimerOne.h>
#include <LiquidCrystal.h>

#define DHTTYPE DHT11   // DHT 11
#define DHT11_PIN 13
#define LED_PIN1 10// red
#define LED_PIN2 5 // green 
#define TRIGGER_PIN 8
#define ECHO_PIN 7
int X;        // variable para almacenar valor leido del eje X
int Y;        // variable para almacenar valor leido del eje y
#define PULSADOR 9    // pulsador incorporado pin digital 9
int SW;       // variable para almacenar valor leido del pulsador
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
DHT dht(DHT11_PIN, DHTTYPE);
//ThreadController controller = ThreadController();

bool start_state = false;
bool service_state = false;
bool admin_state = false;

int ledstate = LOW;
int counter_led1 = 0;
//int distance_sensor = 0;

void show_temp_hum(){
  // Wait a few seconds between measurements.
  delay(2000); // fix this

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
  delay(1000);
  
}
//return distance in cm
int get_distance(){

  float time;
  float distance;
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10); //fix this
  digitalWrite(TRIGGER_PIN, LOW);
  time=pulseIn(ECHO_PIN, HIGH);
  
  Serial.println(time);  

  //distance = ((time * 0.3432) / 2);
  distance = time / 29 / 2; // I do not know why 

  Serial.println(distance);
  return distance;
}

void blinkLED() {
  
  counter_led1++;
  ledstate = !ledstate;
}

void setup() {
  //config pins
  pinMode(LED_PIN1, OUTPUT);
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  lcd.begin(LCD_COLS,LCD_ROWS);

  // make sure led is off
  digitalWrite(LED_PIN1, LOW);

  //start interruption
  Timer1.initialize(500000);
  Timer1.attachInterrupt(blinkLED);
  start_state = true;
  Serial.begin(9600);

  dht.begin();
}

void loop() {

  if (start_state){
    if(counter_led1 < 8){
      // for the lcd not move letters
      lcd.setCursor(3,0);
      lcd.print("CARGANDO...");
      digitalWrite(LED_PIN1, ledstate);
    }else if (counter_led1 == 8 ){
      lcd.clear();

    }else if (counter_led1 == 9 || counter_led1 == 10 || counter_led1 == 11 ){
      lcd.setCursor(3,0);
      lcd.print("Servicio");
    }else{
      lcd.clear();
      service_state = true;
      start_state = false;
    }      
  }

  if(service_state){

    int distance_sensor = 0;

    distance_sensor = get_distance();
    if (0 < distance_sensor && distance_sensor < 100){
      show_temp_hum();
      Serial.println("I AM CLOSE");
    }else{
      lcd.clear();
      lcd.setCursor(3,0);
      lcd.print("ESPERANDO");
      lcd.setCursor(4,1);
      lcd.print("CLIENTE");
    }
  }
}



