
void sub_menu_secs(){

  lcd.setCursor(6, 0);
  lcd.print(millis()/SEC2MS);
  lcd.print("s");
}

void sub_menu_prices(){
          
  now_state_y = analogRead(Y_AXIS);

  if (now_state_y < UP){
    if(arr_pos_2 > MIN_POS){
      arr_pos_2--;
      lcd.clear();
    }
  }
  if (now_state_y > DOWN){
    if(arr_pos_2 < 4){
      arr_pos_2++;
      lcd.clear();
    }
  }

  lcd.setCursor(0, 0);
  lcd.print(coffees[arr_pos_2]);
  lcd.setCursor(0, 1); 
  lcd.print(prices[arr_pos_2]);
  lcd.write(VAR_EURO);

  now_state_x = analogRead(X_AXIS);

  if (now_state_x > RIGHT){
    lcd.setCursor(0, 0);
    lcd.print(coffees[arr_pos_2]);
    changing_prices = true;
  }
}
// show menu list in admin state
void show_list(){

  if ((millis() - prev_time) > ONE_HUNDRED_FIFTY_MS){

    // use y axis for scrolling the lcd 
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
    
    // shows options in lcd 
    lcd.setCursor(0, 0);
    lcd.print(menu[arr_pos]);
    lcd.setCursor(0, 1);
    lcd.print(menu2[arr_pos]);

    unsigned int joy_button = digitalRead(SW_BUTTON);
    // if the button is pressed enter to one
    // of the sub menus
    if (joy_button == PRESSED){

      lcd.clear();
      sub_menus = true;
    }    
    prev_time = millis();  
  }
}

// increments counter of the random time
// preparing coffee
void preparing_coffee(){
  coffee_time++;
}

//show products of the service state 
void show_products(){

  if ((millis() - prev_time) > ONE_HUNDRED_FIFTY_MS){

    now_state_y = analogRead(Y_AXIS);

    // used y axis for scrolling  the products in lcd
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

    // shows products in lcd 
    lcd.setCursor(0, 0);
    lcd.print(coffees[arr_pos]);
    lcd.setCursor(0, 1); 
    lcd.print(prices[arr_pos]);
    // print €
    lcd.write(VAR_EURO);

    unsigned int joy_button = digitalRead(SW_BUTTON);

    // if button is pressed, enter 
    // to the phase one of prepare coffee

    if (joy_button == PRESSED){

      lcd.clear();
      //get random number between 4 to 8
      random_num = random(MIN_RND,MAX_RND);
      prepare_coffee = true;
      phase_one= true;
    }
    prev_time = millis();
  }
}

//increments the counter for showing
// temp and hum in service state
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

// is an ISR and changes the state and counter
// for making the led blink
// this is used in start state 
void blinkLED() {
  
  counter_led1++;
  ledstate = !ledstate;
}




