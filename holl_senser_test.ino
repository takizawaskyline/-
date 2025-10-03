#define HOL_PIN A0 

void setup() {
  pinMode(HOL_PIN, INPUT);
  Serial.begin(9600);

}

void loop() {
  int sensorValue = analogRead(HOL_PIN);  // Более описательное имя переменной
  Serial.println(sensorValue);
  delay(100);  
}
