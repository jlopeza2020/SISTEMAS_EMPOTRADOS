#include <StaticThreadController.h>
#include <Thread.h>
#include <ThreadController.h>

#include <Adafruit_Sensor.h>
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

// threads 
ThreadController controller = ThreadController();
Thread distanceThread = Thread();
Thread  hum_tempThread = Thread();

bool start_state = false;
bool service_state = false;
bool admin_state = false;
bool detected_person = false;

int ledstate = LOW;
int counter_led1 = 0;
int counter_t_h = 0;


String coffees[] = {"Cafe solo", "Cafe Cortado", "Cafe Doble", "Cafe Premium", "Chocolate"};
float prices[] = {1, 1.10, 1.25, 1.50, 2.00};

byte euro_symbol[8] = {
  0b00110,
  0b01001,
  0b01000,
  0b11110,
  0b11110,
  0b01000,
  0b01001,
  0b00110
};


void show_products(){

  //lcd.clear();

  lcd.createChar(0, euro_symbol); // create a new custom character
  lcd.setCursor(2, 0); // move cursor to (2, 0)
  lcd.write((byte)0);  // print the custom char at (2, 0)
}


void show_t_h(){

  counter_t_h++;
}
void callback_hum_dist_thread(){
  float hum = dht.readHumidity();
  // Read temperature as Celsius
  float temp = dht.readTemperature();

  // Check if any read failes
  if (isnan(hum) || isnan(temp)) {
    Serial.println(F("Failed reading from DHT sensor!"));
    return;
  }

  //Serial.print(F(" Humidity: "));
  //Serial.print(hum);
  //Serial.print(F("%  Temperature: "));
  //Serial.print(temp);
  //Serial.print(F("ºC "));
  
  lcd.setCursor(0,0); 
  lcd.print("Temp: ");
  lcd.print(temp);
  lcd.print((char)223);
  lcd.print("C");
  lcd.setCursor(0,1);
  lcd.print("Humidity: ");
  lcd.print(hum);
  lcd.print("%");
}

void callback_dist_thread(){

  int distance_sensor = 0;

  distance_sensor = get_distance();
  if (0 < distance_sensor && distance_sensor < 100){
    detected_person = true;  
  }else{
    lcd.clear();
    lcd.setCursor(3,0);
    lcd.print("ESPERANDO");
    lcd.setCursor(4,1);
    lcd.print("CLIENTE");
  }

}

//return distance in cm
int get_distance(){

  float time;
  float distance;
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);
  time=pulseIn(ECHO_PIN, HIGH);
  
  distance = time / 29 / 2; 
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
  // 500 milis 
  Timer1.initialize(500000);
  Timer1.attachInterrupt(blinkLED);
  start_state = true;

  // THREADS SECTION 
  // distance thread
  distanceThread.enabled = true;
  distanceThread.setInterval(300);
  distanceThread.onRun(callback_dist_thread);

  //hum thread 
  hum_tempThread.enabled = true;
  hum_tempThread.setInterval(250);
  hum_tempThread.onRun(callback_hum_dist_thread);

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
      // this interruption we won't use it again so I disable it
      detachInterrupt(digitalPinToInterrupt(LED_PIN1));
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
    controller.add(&distanceThread);

    // show durante 5 segundos 
    if(detected_person){
      controller.remove(&distanceThread);
      //one second
      Timer1.setPeriod(1000000);
      Timer1.attachInterrupt(show_t_h);
      //controller.remove(&distanceThread);
      if(counter_t_h < 5){
 
        callback_hum_dist_thread();
      }else if(counter_t_h == 5){
        lcd.clear();

      }else{
        detachInterrupt(digitalPinToInterrupt(DHT11_PIN));
        show_products();
      }
    }
  }

  //controller.add(&hum_tempThread); // better use in admin mode

  controller.run();
}



