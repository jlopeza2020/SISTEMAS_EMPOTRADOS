
void preparing_coffee(){
  coffee_time++;
}

void show_products(){

  if ((millis() - prev_time) > 150){

    now_state_y = analogRead(Y_AXIS);

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
    lcd.write(VAR_EURO);

    //manage joystick button
    unsigned int joy_button = digitalRead(SW_BUTTON);
    if (joy_button == 0){

      lcd.clear();
      //get random number between 4 to 9 
      random_num = random(4,9);
      prepare_coffee = true;
      phase_one= true;
    }
  
    prev_time = millis();

  }
}

void show_t_h(){

  counter_t_h++;
}

void hum_temp(){
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


//return distance in cm
int get_distance(){

  float time;
  float distance;
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);
  time=pulseIn(ECHO_PIN, HIGH);
  
  distance = time / 29 / 2; 
  return distance;
}

void blinkLED() {
  
  counter_led1++;
  ledstate = !ledstate;
}




