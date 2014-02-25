
const int BUFFER_SIZE = 100;
char buffer[BUFFER_SIZE];

int action(char *str)
{
  if(strcmp(str, "bell") == 0)
  {
    Serial.println("Bell action.");
    digitalWrite(3, HIGH);
    delay(500);
    digitalWrite(3, LOW);
    return 1;
  }
  else
    Serial.println("Unrecognised command.\n");
    return 0;
}

void setup()
{
  memset(buffer, '\0', sizeof(buffer));
  
  pinMode(3, OUTPUT); /* Solenoid PWM */
  pinMode(12, OUTPUT); /* Solenoid Direction */
  digitalWrite(12, HIGH);
  
  Serial.begin(115200);
}

void loop()
{
  if( Serial.available() )
  {
    int nread = Serial.readBytesUntil('\n', buffer, BUFFER_SIZE - 1);
    //buffer[nread - 1] = '\0'; /* strip newline */
    //Serial.println(buffer);
    action(buffer);
    memset(buffer, '\0', nread);
  }
}
