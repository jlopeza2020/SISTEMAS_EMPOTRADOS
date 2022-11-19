#include <StaticThreadController.h>
#include <Thread.h>
#include <ThreadController.h>

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#include <TimerOne.h>
#include <LiquidCrystal.h>

// DHT 11 sensor 
#define DHTTYPE DHT11  
#define DHT11_PIN 13

// red led
#define LED_PIN1 10
// green
#define LED_PIN2 5 

//ultrasonic sensor 
#define TRIGGER_PIN 8
#define ECHO_PIN 7

//Joystick 
#define X_AXIS A1       
#define Y_AXIS A0     
#define SW_BUTTON 9 

#define BUTTON 1; 

//lcd structure
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

//global vars
bool start_state = false;
bool service_state = false;
bool admin_state = false;
bool detected_person = false;
bool is_pressed = false;
bool prepare_coffee = false;
bool phase_one = false;
bool phase_two = false;

int ledstate = LOW;
int counter_led1 = 0;
int counter_t_h = 0;
int now_state_y = 0;
int arr_pos = 0;
int coffee_time = 0;
int random_num = 0;
unsigned long int prev_time;
unsigned long int prev_time2;


String coffees[] = {"Cafe solo", "Cafe Cortado", "Cafe Doble", "Cafe Premium", "Chocolate"};
float prices[] = {1, 1.10, 1.25, 1.50, 2.00};
float distancia;


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

void preparing_coffee(){
  coffee_time++;
}
void show_products(){

  now_state_y = analogRead(Y_AXIS);

  if ((millis() - prev_time) > 150){

    if (now_state_y < 100){
      if(arr_pos > 0){
        arr_pos--;
        lcd.clear();
      }
    }
    if (now_state_y > 900){
      if(arr_pos < 4){
        arr_pos++;
        lcd.clear();
      }
    }

    lcd.setCursor(0, 0);
    lcd.print(coffees[arr_pos]);
    lcd.setCursor(0, 1); 
    lcd.print(prices[arr_pos]);
    // print €
    lcd.write(3);

    //manage joystick button
    unsigned int joy_button = digitalRead(SW_BUTTON);
    if (joy_button == 0){
      //is_pressed = true;
      lcd.clear();
      random_num = random(4,9); // from 4 to 8
      prepare_coffee = true;
      phase_one= true;
    }
  
    prev_time = millis();

  }
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

  lcd.setCursor(0,0); 
  lcd.print("Temp: ");
  lcd.print(temp);
  lcd.print((char)223);
  lcd.print("C");
  lcd.setCursor(0,1);
  lcd.print("Humedad: ");
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
  pinMode(X_AXIS, INPUT);
  pinMode(Y_AXIS, INPUT);
  pinMode(SW_BUTTON, INPUT_PULLUP);

  lcd.begin(LCD_COLS,LCD_ROWS);

  // make sure led is off
  digitalWrite(LED_PIN1, LOW);

  //start interruption
  // 500 milis 
  Timer1.initialize(500000);
  Timer1.attachInterrupt(blinkLED);

  //Timer0.initialize(1000000);
  start_state = true;

  // THREADS SECTION 
  // distance thread
  distanceThread.enabled = true;
  distanceThread.setInterval(300);
  distanceThread.onRun(callback_dist_thread);

  //hum thread use in admin mode 
  hum_tempThread.enabled = true;
  hum_tempThread.setInterval(250);
  hum_tempThread.onRun(callback_hum_dist_thread);

  Serial.begin(9600);
  dht.begin();
  // create custom € symbol
  lcd.createChar(3, euro_symbol);
  //random_num = random(4,9); // from 4 to 8
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
      if(counter_t_h < 5){
 
        callback_hum_dist_thread();
      }else if(counter_t_h == 5){
        lcd.clear();

      }else{
        detachInterrupt(digitalPinToInterrupt(DHT11_PIN));
        if(prepare_coffee != true){
          show_products();          
        }
        
      }
    }
    if(prepare_coffee){
  
      if(phase_one){
        if ((millis() - prev_time2) > random_num*1000){
          Serial.println(millis() - prev_time2);
          lcd.setCursor(3,0);
          lcd.print("Preparando");
          lcd.setCursor(4,1);
          lcd.print("Cafe ...");
          //treat led2 
          prev_time2 = millis();
        }else if ((millis() - prev_time2) ==  random_num*1000){
          lcd.clear();
          phase_one = false;
          phase_two = true;
          prev_time2 = 0;
        }
      }
      if(phase_two){

        if ((millis() - prev_time2) > 3000){
          Serial.println(millis() - prev_time2);
          lcd.setCursor(4,0);
          lcd.print("RETIRE");
          lcd.setCursor(4,1);
          lcd.print("BEBIDA");
          prev_time2 = millis();
          //treat led2

        }else if ((millis() - prev_time2) ==  3000){
          lcd.clear();
          phase_two = false;
          prev_time2 = 0;

        }
      } 
    }
  }
  controller.run();
}



