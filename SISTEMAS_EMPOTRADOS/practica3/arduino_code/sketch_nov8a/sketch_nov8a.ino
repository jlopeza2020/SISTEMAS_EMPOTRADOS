
int LED_PIN1 = 3;// red
int LED_PIN2 = 5; // green 
int TRIGGER_PIN = 8;
int ECHO_PIN = 7;
float distancia;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  //pinMode(LED_PIN1, OUTPUT);
  //pinMode(LED_PIN2, OUTPUT);
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

}


void loop() {
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);
  distancia=pulseIn(ECHO_PIN, HIGH);
  Serial.println(distancia);


  //analogWrite(LED_PIN1, 50);
  //delay(1000);
}