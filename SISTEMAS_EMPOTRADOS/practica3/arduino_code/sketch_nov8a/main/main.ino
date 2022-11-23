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
// green led
#define LED_PIN2 5 

//ultrasonic sensor 
#define TRIGGER_PIN 8
#define ECHO_PIN 7

//Joystick 
#define X_AXIS A1       
#define Y_AXIS A0     
#define SW_BUTTON 9 

#define BUTTON A2

//lcd structure
#define RS 12
#define ENABLE 11
#define D0 6 
#define D1 4
#define D2 3 
#define D3 2
#define LCD_COLS 16
#define LCD_ROWS 2

//VALUES
//#define SEC 1000

// in miliseconds 
#define THREE_HUNDRED_MS 300
#define TWO_HUNDRED_FIFTY_MS 250
#define VAR_EURO 3
#define ONE_HUNDRED_FIFTY_MS 150
#define FIVE_S2MS 5000
#define TWO_S2MS 2000
#define THREE_S2MS 3000
#define SEC2MS 1000

//in microseconds
#define FIVE_HUNDRED_MS 500000
#define ONE_SEC 1000000

// SECONDS
#define FIVE_S 5
#define SIX_S 6
// LED1 seconds
#define MAX_LED1_S 8 
#define MAX_START_S 11 

// joystick axis
// y axis
#define UP 100
#define DOWN 900
// x axis
#define LEFT 100

// button (either joystick or the other)
#define PRESSED 0

//random values
#define MIN_RND 4
#define MAX_RND 9

//ultrasonic sensor 
#define MAX_DIST_PERSON 100
#define MIN_DIST_PERSON 0



LiquidCrystal lcd(RS, ENABLE, D0, D1, D2, D3);
DHT dht(DHT11_PIN, DHTTYPE);

// threads
ThreadController controller = ThreadController();
Thread distanceThread = Thread();
Thread  hum_tempThread = Thread();
Thread shine_led = Thread();
Thread button_pressed = Thread();
Thread admin = Thread();
Thread out_admin = Thread();

//global vars
bool start_state = false;
bool service_state = false;
bool admin_state = false;
bool detected_person = false;
bool prepare_coffee = false;
bool phase_one = false;
bool phase_two = false;
bool changing_prices = false;

int ledstate = LOW;
int counter_led1, counter_t_h, now_state_y, arr_pos, coffee_time, random_num, led_value;
unsigned long int prev_time;
unsigned long int prev_time2;
long lastTimeTemp = 0;
int count = 0;
int now_state_x;
int counter_button;
unsigned long int time;
unsigned long int time2;
unsigned long int time3;

String coffees[] = {"Cafe solo", "Cafe Cortado", "Cafe Doble", "Cafe Premium", "Chocolate"};
float prices[] = {1, 1.10, 1.25, 1.50, 2.00};
float initial_prices[] = {1, 1.10, 1.25, 1.50, 2.00};
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

void setup() {
  //config pins
  pinMode(LED_PIN1, OUTPUT);
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(X_AXIS, INPUT);
  pinMode(Y_AXIS, INPUT);
  pinMode(SW_BUTTON, INPUT_PULLUP);

  lcd.begin(LCD_COLS,LCD_ROWS);

  // make sure leds are off
  digitalWrite(LED_PIN1, LOW);
  analogWrite(LED_PIN2, LOW);

  //interruption 
  Timer1.initialize(FIVE_HUNDRED_MS);
  Timer1.attachInterrupt(blinkLED);

  start_state = true;

  // THREADS SECTION 

  // distance thread
  distanceThread.enabled = true;
  distanceThread.setInterval(THREE_HUNDRED_MS);
  distanceThread.onRun(callback_dist_thread);

  //shine led
  shine_led.enabled = true;
  shine_led.setInterval(SEC2MS);
  shine_led.onRun(callback_led_shine);

  //button_pressed
  button_pressed.enabled = true;
  button_pressed.setInterval(TWO_HUNDRED_FIFTY_MS);
  button_pressed.onRun(callback_service_button);

  //in admin mode
  admin.enabled = true;
  admin.setInterval(TWO_HUNDRED_FIFTY_MS);
  admin.onRun(callback_admin_mode);

  //out admin mode 
  out_admin.enabled = true;
  out_admin.setInterval(TWO_HUNDRED_FIFTY_MS);
  out_admin.onRun(callback_out_admin_mode);

  
  Serial.begin(9600);
  dht.begin();
  // create custom € symbol
  lcd.createChar(VAR_EURO, euro_symbol);

  //set random seed
  randomSeed(analogRead(A0));

}

void loop() {

  controller.add(&admin);

  if (start_state){
    if(counter_led1 < MAX_LED1_S){
      // for the lcd not move letters
      lcd.setCursor(3,0);
      lcd.print("CARGANDO...");
      digitalWrite(LED_PIN1, ledstate);
    }else if (counter_led1 == MAX_LED1_S){
      // this interruption we won't use it again so I disable it
      detachInterrupt(digitalPinToInterrupt(LED_PIN1));
      lcd.clear();

    }else if (counter_led1 >= MAX_LED1_S && counter_led1  <= MAX_START_S){
      lcd.setCursor(3,0);
      lcd.print("Servicio");
    }else{
      lcd.clear();
      service_state = true;
      start_state = false;
    }      
  }

  if(service_state){ // continous loop 

    //int distance_sensor = 0;
    controller.add(&distanceThread);

    // b phase
    if(detected_person){

      // if button is pressed between 2-3 restart service state
      controller.add(&button_pressed);

      controller.remove(&distanceThread);
      //one second
      Timer1.setPeriod(ONE_SEC);
      Timer1.attachInterrupt(show_t_h);

      if(counter_t_h <= FIVE_S){
 
        hum_temp();      
      }else if(counter_t_h == SIX_S){
        lcd.clear();
        prepare_coffee = false;

      }else{
        detachInterrupt(digitalPinToInterrupt(DHT11_PIN));
        if(prepare_coffee != true){
          show_products();          
        }
        
      }

      if(prepare_coffee){
      
        if(phase_one){ 
               
          if (millis() - lastTimeTemp > SEC2MS){

            lastTimeTemp = millis();
            count++;
    
          }
          if (count <= random_num){

            lcd.setCursor(3,0);
            lcd.print("Preparando");
            lcd.setCursor(4,1);
            lcd.print("Cafe ...");

            controller.add(&shine_led);

          }else{
            lcd.clear();
            analogWrite(LED_PIN2, LOW);
            controller.remove(&shine_led);
            phase_one = false;
            phase_two = true;
            lastTimeTemp = 0;
            count = 0;
        
          }
        }
        if(phase_two){
        

          if (millis() - lastTimeTemp > SEC2MS){

            lastTimeTemp = millis();
            count++;
    
          }
          if (count <= 3){

            lcd.setCursor(4,0);
            lcd.print("RETIRE");
            lcd.setCursor(4,1);
            lcd.print("BEBIDA");

          }else{

            lcd.clear();
            phase_two = false;
            detected_person = false;
            lastTimeTemp = 0;
            count = 0;
            led_value = 0;
          }

        }
      
      }

    }

  }

  if (admin_state){

    controller.remove(&admin);
    controller.add(&out_admin);    
    digitalWrite(LED_PIN1, HIGH);
    analogWrite(LED_PIN2, 255);

    float hum = dht.readHumidity();

    // Read temperature as Celsius
    float temp = dht.readTemperature();
  
    if ((millis() - prev_time) > ONE_HUNDRED_FIFTY_MS){

      now_state_y = analogRead(Y_AXIS);

      if (now_state_y < UP){
        if(arr_pos > 0){
          arr_pos--;
          lcd.clear();
        }
      }
      if (now_state_y > DOWN){
        if(arr_pos < 6){
          arr_pos++;
          lcd.clear();
        }
      }
      
      if(arr_pos == 0){

        lcd.setCursor(0, 0);
        lcd.print("Temp:");
        lcd.print(int(temp));
        lcd.print("C");
        lcd.print(" ");
        lcd.print("Hum:");
        lcd.print(int(hum));
        lcd.print("%");

        lcd.setCursor(0, 1);
        lcd.print("Distancia: ");
        lcd.print(get_distance());
        lcd.print("cm");

      }else if (arr_pos == 1){

        lcd.setCursor(0, 0);
        lcd.print("Distancia: ");
        lcd.print(get_distance());
        lcd.print("cm");

        lcd.setCursor(0, 1);
        lcd.print(millis()/1000);
        lcd.print("s");
      }else{
        lcd.setCursor(0, 0);
        lcd.print(coffees[arr_pos-2]);
        lcd.setCursor(0, 1); 
        lcd.print(prices[arr_pos-2]);
        lcd.write(VAR_EURO);
      }

      unsigned int joy_button = digitalRead(SW_BUTTON);
      if (joy_button == PRESSED){

        lcd.setCursor(0, 0);
        lcd.print(coffees[arr_pos-2]);
        changing_prices = true;

      }

      prev_time = millis();
    
    }

    if (changing_prices){

      Serial.println("Im in");
      if ((millis() - prev_time) > ONE_HUNDRED_FIFTY_MS){
        now_state_y = analogRead(Y_AXIS);

        if (now_state_y < UP){
          lcd.setCursor(0, 1); 
          lcd.print(prices[arr_pos-2] += 0.05);
          lcd.write(VAR_EURO);
        }
        if (now_state_y > DOWN){
          if (prices[arr_pos-2] >= 0.00){
            lcd.setCursor(0, 1); 
            lcd.print(prices[arr_pos-2] -= 0.05);
            lcd.write(VAR_EURO);
          }
        }

        unsigned int joy_button = digitalRead(SW_BUTTON);
        if (joy_button == PRESSED){

          changing_prices = false;
        }

        now_state_x = analogRead(X_AXIS);

        Serial.println(now_state_x);

        if (now_state_x < LEFT){

          prices[arr_pos-2] = initial_prices[arr_pos -2];
          changing_prices = false;
        }
        

        prev_time = millis();

      }


    }

  }
  
  controller.run();  
}



