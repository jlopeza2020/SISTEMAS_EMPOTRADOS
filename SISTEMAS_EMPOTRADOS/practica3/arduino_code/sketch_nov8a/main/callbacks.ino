void callback_sub_menu_temp(){
  float hum = dht.readHumidity();

  // Read temperature as Celsius
  float temp = dht.readTemperature();

  lcd.setCursor(0, 0);
  lcd.print("Temp:");
  lcd.print(int(temp));
  lcd.print((char)223);
  lcd.print("C");
  lcd.print("Hum:");
  lcd.print(int(hum));
  lcd.print("%");

}

void callback_sub_menu_dist(){
  lcd.setCursor(0, 0);
  lcd.print("Distancia: ");
  lcd.print(get_distance());
  lcd.print("cm");
}



//check if the button is pressed more that five secs
// in admin mode and get out of it. Returns to 
// the service state  
void callback_out_admin_mode(){

  unsigned int button_state = digitalRead(BUTTON);

  if(button_state == PRESSED) {
    time=millis();
    while(button_state == PRESSED) {
      time2=millis();
      button_state= digitalRead(BUTTON);
    }
    time3=time2-time;

  }

  //gets out of admin mode 
  if (time3 >= FIVE_S2MS){
    lcd.clear();
    digitalWrite(LED_PIN1, LOW);
    analogWrite(LED_PIN2, LOW);
    admin_state = false;
    service_state = true;
    time3 = 0;
    
  }

}

//check if the button is pressed more that five secs
// in any part of the start or service states and 
// gets into it 
void callback_admin_mode(){

  controller.remove(&out_admin);
  unsigned int button_state = digitalRead(BUTTON);

  if(button_state == PRESSED) {
    time=millis();
    while(button_state == PRESSED) {
      time2=millis();
      button_state= digitalRead(BUTTON);
    }
    time3=time2-time;

  }

  // admin_mode 
  if (time3 >= FIVE_S2MS){

    start_state = false;
    service_state = false;
    lcd.clear();
    counter_t_h = 0;
    prepare_coffee = false;
    phase_one = false;
    count = 0;
    phase_two = false;
    lastTimeTemp = 0;
    controller.remove(&distanceThread);
    detachInterrupt(digitalPinToInterrupt(DHT11_PIN));
    controller.remove(&shine_led);
    analogWrite(LED_PIN2, LOW);
    detected_person = false;
    controller.remove(&distanceThread);
    time3 = 0;
    controller.remove(&callback_service_button);
    admin_state = true;
  }
}

// if the button is pressed between 2 to 3 
// secs, comes back to the service state
// phase b) 
void callback_service_button(){

  unsigned int button_state = digitalRead(BUTTON);

  if(button_state == PRESSED) {
    time=millis();
    while(button_state == PRESSED) {
      time2=millis();
      button_state= digitalRead(BUTTON);
    }
    time3=time2-time;

  }

  // restart service state 
  if (time3 >= TWO_S2MS && time3 <= THREE_S2MS){
    counter_t_h = 0;
    prepare_coffee = false;
    phase_one = false;
    count = 0;
    phase_two = false;
    lastTimeTemp = 0;
    controller.remove(&distanceThread);
    detachInterrupt(digitalPinToInterrupt(DHT11_PIN));
    controller.remove(&shine_led);
    analogWrite(LED_PIN2, LOW);
    detected_person = false;
    controller.remove(&distanceThread);
    time3 = 0;
    controller.remove(&callback_service_button);
  }
}

// makes the led shine incrementally 
// used in prepare coffee
void callback_led_shine(){
  led_value += MAX_ANALOG_VALUE/random_num;
  analogWrite(LED_PIN2, led_value);
}

// if looking if a person is 
// less than 1 meter closer 
// else: prints "ESPERANDO CLIENTE"
void callback_dist_thread(){

  int distance_sensor = 0;

  distance_sensor = get_distance();
  if (MIN_DIST_PERSON < distance_sensor && distance_sensor < MAX_DIST_PERSON){

    detected_person = true;
    counter_t_h = 0; 
  }else{
    lcd.clear();
    lcd.setCursor(3,0);
    lcd.print("ESPERANDO");
    lcd.setCursor(4,1);
    lcd.print("CLIENTE");
  }
}




