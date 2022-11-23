
void callback_out_admin_mode(){

  unsigned int button_state = digitalRead(BUTTON);

  if(button_state == 0) {
    time=millis();
    while(button_state == 0) {
      time2=millis();
      button_state= digitalRead(BUTTON);
    }
    time3=time2-time;

    Serial.println("out");
    Serial.println(time3);

  }
  //out admin mode 
  if (time3 >= 5000){
    Serial.println("out");    
    lcd.clear();
    digitalWrite(LED_PIN1, LOW);
    analogWrite(LED_PIN2, 0);
    admin_state = false;
    service_state = true;
    time3 = 0;
    
  }

}
void callback_admin_mode(){

  controller.remove(&out_admin);
  unsigned int button_state = digitalRead(BUTTON);

  if(button_state == 0) {
    time=millis();
    while(button_state == 0) {
      time2=millis();
      button_state= digitalRead(BUTTON);
    }
    time3=time2-time;

    Serial.println(time3);

  }

  // admin_mode 
  if (time3 >= 5000){
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
    analogWrite(LED_PIN2, 0);
    detected_person = false;
    controller.remove(&distanceThread);
    time3 = 0;
    controller.remove(&callback_service_button);
    admin_state = true;
  }
}

void callback_service_button(){

  unsigned int button_state = digitalRead(BUTTON);

  if(button_state == 0) {
    time=millis();
    while(button_state == 0) {
      time2=millis();
      button_state= digitalRead(BUTTON);
    }
    time3=time2-time;

    Serial.println(time3);

  }

  // restart service state 
  if (time3 >= 2000 && time3 <= 3000){
    counter_t_h = 0;
    prepare_coffee = false;
    phase_one = false;
    count = 0;
    phase_two = false;
    lastTimeTemp = 0;
    controller.remove(&distanceThread);
    detachInterrupt(digitalPinToInterrupt(DHT11_PIN));
    controller.remove(&shine_led);
    analogWrite(LED_PIN2, 0);
    detected_person = false;
    controller.remove(&distanceThread);
    time3 = 0;
    controller.remove(&callback_service_button);
  }
}

void callback_led_shine(){
  led_value += 255/random_num;
  analogWrite(LED_PIN2, led_value);
}

void callback_dist_thread(){

  int distance_sensor = 0;

  distance_sensor = get_distance();
  if (0 < distance_sensor && distance_sensor < 100){
    Serial.println("dentro");
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




