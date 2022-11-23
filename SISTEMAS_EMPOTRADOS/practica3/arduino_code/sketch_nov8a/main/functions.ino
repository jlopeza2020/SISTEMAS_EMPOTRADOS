void show_list(){

  if ((millis() - prev_time) > ONE_HUNDRED_FIFTY_MS){

    now_state_y = analogRead(Y_AXIS);

    if (now_state_y < UP){
      if(arr_pos > MIN_POS){
        arr_pos--;
        lcd.clear();
      }
    }
    if (now_state_y > DOWN){
      if(arr_pos < 3){
        arr_pos++;
        lcd.clear();
      }
    }
    

    lcd.setCursor(0, 0);
    lcd.print(menu[arr_pos]);
    lcd.setCursor(0, 1);
    lcd.print(menu2[arr_pos]);

    unsigned int joy_button = digitalRead(SW_BUTTON);
    if (joy_button == PRESSED){
      Serial.println("he sido pulsado");

      lcd.clear();
      //get random number between 4 to 8
      //random_num = random(MIN_RND,MAX_RND);
      //prepare_coffee = true;
      //phase_one= true;
      //arr_pos_2 = 0;
      sub_menus = true;

    }    

    /*if(arr_pos == 0){

      lcd.setCursor(0, 0);
      lcd.print("Temp:");
      lcd.print(int(temp));
      lcd.print((char)223);
      lcd.print("C");
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
      lcd.print(millis()/SEC2MS);
      lcd.print("s");
    }else{
      lcd.setCursor(0, 0);
      lcd.print(coffees[arr_pos-2]);
      lcd.setCursor(0, 1); 
      lcd.print(prices[arr_pos-2]);
      lcd.write(VAR_EURO);
    }

    now_state_x = analogRead(X_AXIS);

    if (now_state_x > RIGHT){
      lcd.setCursor(0, 0);
      lcd.print(coffees[arr_pos-2]);
      changing_prices = true;
      
    }  
    //unsigned int joy_button = digitalRead(SW_BUTTON);
    //Serial.println(joy_button);
    //if (joy_button == PRESSED){

    //lcd.setCursor(0, 0);
    //  lcd.print(coffees[arr_pos-2]);
    //  changing_prices = true;

    //}*/

    prev_time = millis();
    
  }
}

void preparing_coffee(){
  coffee_time++;
}

void show_products(){

  if ((millis() - prev_time) > ONE_HUNDRED_FIFTY_MS){

    now_state_y = analogRead(Y_AXIS);

    if (now_state_y < UP){
      if(arr_pos > MIN_POS){
        arr_pos--;
        lcd.clear();
      }
    }
    if (now_state_y > DOWN){
      if(arr_pos < MAX_POS_INIT_LIST){
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
    //Serial.println();digitalRead(SW_BUTTON)

    if (joy_button == PRESSED){
      //Serial.println("he sido pulsado");

      lcd.clear();
      //get random number between 4 to 8
      random_num = random(MIN_RND,MAX_RND);
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
  delayMicroseconds(TEN_MICROS);
  digitalWrite(TRIGGER_PIN, LOW);
  time=pulseIn(ECHO_PIN, HIGH);
  
  // conversion into cm 
  distance = time / 29 / 2; 
  return distance;
}

void blinkLED() {
  
  counter_led1++;
  ledstate = !ledstate;
}




