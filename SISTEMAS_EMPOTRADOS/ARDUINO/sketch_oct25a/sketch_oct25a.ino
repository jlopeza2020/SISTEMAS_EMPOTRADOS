
#define SLEEP_TIME_MILIS 10
#define SLEEP_TIME_MICRO 100

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

}

void loop() {
  // put your main code here, to run repeatedly:
  int tic = millis();
  delay(SLEEP_TIME_MILIS);
  int toc = millis();

  int time = (toc - tic) - SLEEP_TIME_MILIS;
  Serial.println(time);

  //NO ES REPRESENTATIVA LA INFO QUE ESTAMOS TENIENDO
  //LA RESOLUCIÓN ES DE 1 

  int tic_2 = micros();
  delayMicroseconds(SLEEP_TIME_MILIS);
  int toc_2 = micros();

  int time_2 = (toc - tic) - SLEEP_TIME_MILIS;
  
  
  Serial.println("Latencia (ms): " + String(time) + "- Latencia (micros)" +  String(time_2));

  //la resolucion es lo que cambia 
  // si trabaja con micros su resolución no sea 1, es por la arquitectura 
  // mínimo es 4 
  // no mide 1 micro, sino mide de 4 en 4 
  // cuanto mayor es el delay mayor es el error 

  // la resolucion es de 4 (mínimo vaalor que puede medir)
  // la precisión es baja porque la dispersión es baja (poca diferencia entre valores)
  // no es exacto, no se despierta en el valor deseado 

  // los delays en microsegundos son mayores que el rango esperado, hay buffer overflowsss

}
