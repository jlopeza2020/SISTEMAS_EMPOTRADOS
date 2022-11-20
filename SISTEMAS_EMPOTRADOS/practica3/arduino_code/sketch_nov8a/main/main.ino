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
Thread shine_led = Thread();

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
int led_value = 0;
int i = 0;
//unsigned long int time;
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

void callback_led_shine(){
  led_value += 255/random_num;
  Serial.println(led_value);
  analogWrite(LED_PIN2, led_value);  
}

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
  //Serial.println(distance);
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
  analogWrite(LED_PIN1, 0);

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

  //shine led
  shine_led.enabled = true;
  shine_led.setInterval(1000);
  shine_led.onRun(callback_led_shine);

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
        //i = (millis() - prev_time2);
        //Serial.println(i);
        
        /*if ( i == 990 || i == 991 || i == 992 || i == 993 || i == 994 || i == 995 || i == 996 || i == 997 || i == 998 || i == 999){
          led_value += 255/random_num;
        }else if ( i == 1990 || i == 1991 || i == 1992 || i == 1993 || i == 1994 || i == 1995 || i == 1996 || i == 1997 || i == 1998 || i == 1999){
          led_value += 255/random_num;
        }else if ( i == 2990 || i == 2991 || i == 2992 || i == 2993 || i == 2994 || i == 2995 || i == 2996 || i == 2997 || i == 2998 || i == 2999){
          led_value += 255/random_num;
        }else if (  i == 3990 || i == 3991 || i == 3992 || i == 3993 || i == 3994 || i == 3995 || i == 3996 || i == 3997 || i == 3998 || i == 3999 ){
          led_value += 255/random_num;
        }else if( i == 4990 || i == 4991 || i == 4992 || i == 4993 || i == 4994 || i == 4995 || i == 4996 || i == 4997 || i == 4998 || i == 4999){
          led_value += 255/random_num;
        }else if ( i == 5990 || i == 5991 || i == 5992 || i == 5993 || i == 5994 || i == 5995 || i == 5996 || i == 5997 || i == 5998 || i == 5999 ){
          led_value += 255/random_num;
        }else if (  i == 6990 || i == 6991 || i == 6992 || i == 6993 || i == 6994 || i == 6995 || i == 6996 || i == 6997 || i == 6998 || i == 6999){
          led_value += 255/random_num;
        }else if ( i == 7990 || i == 7991 || i == 7992 || i == 7993 || i == 7994 || i == 7995 || i == 7996 || i == 7997 || i == 7998 || i == 7999){
          led_value += 255/random_num;
        }*/

        
        //Serial.println(led_value);
        //analogWrite(LED_PIN2, led_value);        
        if ((millis() - prev_time2) > random_num*1000){          
          lcd.setCursor(3,0);
          lcd.print("Preparando");
          lcd.setCursor(4,1);
          lcd.print("Cafe ...");
          
          controller.add(&shine_led);

          //analogWrite(LED_PIN2, led_value);        

          //int x = 0;
          //for(int i = 0; i <= random_num*1000; i++){
          //if ( i == 1000 || i == 2000 || i == 3000 ){
              //Serial.println(led_value);
          //led_value += 255/random_num;

              //analogWrite(LED_PIN2, x);
          //}else if ( i == 4000 || i == 5000 || i == 6000){
            //led_value += 255/random_num;

          //}else if ( i == 7000 || i == 8000){
            //led_value += 255/random_num;
          //}
          Serial.println(led_value);
          //analogWrite(LED_PIN2, led_value);
          //Serial.println((millis() - prev_time2));

          //}

          prev_time2 = millis();

        }else if ((millis() - prev_time2) ==  random_num*1000){
          lcd.clear();
          phase_one = false;
          phase_two = true;
          prev_time2 = 0;
          analogWrite(LED_PIN2, 255);
          controller.remove(&shine_led);
        }
      }
      if(phase_two){
        
        analogWrite(LED_PIN2, 0);
        if ((millis() - prev_time2) > 3000){
          //Serial.println(millis() - prev_time2);
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
          //meter watchdog para reiniciar hasta la fun servicio 
        }
      }
    }
  }
  controller.run();
}



