const int sensorPin = A0;    // select the input pin for the potentiometer
const int ledPin = 13;      // select the pin for the LED
const long boudRate = 115200;

const int DisplayXRange = 400;
const char StartCmd = 's';
const char DelayCmd = 'd';
const char TriggerCmd = 't';

int Values[DisplayXRange];
enum eScopeMode { Measure, Idle, WaitTrigger};
enum eTriggerMode { FreeRunning, Positive, Negative};
eScopeMode ScopeMode = Idle;
long inputDelay = 0;
int delayus = 0;
int delayms = 0;
const int InputDelayRange = 10000;
int LedState = 0;
eTriggerMode triggerMode = FreeRunning;
int triggerPrevSample = 0;
int triggerLevel = 0;

void setup() {
  Serial.begin(boudRate);		// Initialize serial communications with the PC
  while (!Serial);
  Serial.setTimeout(1000);
  // declare the ledPin as an OUTPUT:
  pinMode(ledPin, OUTPUT); 
  // analogReference(INTERNAL); // 1.1V
  analogReference(EXTERNAL); // 2.495V
}

void loop() {
  while (Serial.available() > 0) {
    char inByte = Serial.read();
    if (inByte == StartCmd) {
      ScopeMode = Measure;
      Serial.print("$");
    } else if (inByte == DelayCmd) {
      char incomingBytes[10];
      memset(incomingBytes, 0, sizeof(incomingBytes));
      Serial.readBytesUntil(';', incomingBytes, sizeof(incomingBytes) / sizeof(char) - 1);
      inputDelay = atol(incomingBytes);
      inputDelay = (inputDelay > 10000 ? 10000 : inputDelay);
      Serial.print("$d#");
      Serial.print(inputDelay, DEC);
      Serial.println(";:");
      if (inputDelay<InputDelayRange) {
        delayus = inputDelay;
      } else {
        delayms = inputDelay/1000;
      }
    } else if (inByte == TriggerCmd) {
      char incomingBytes[10];
      memset(incomingBytes, 0, sizeof(incomingBytes));
      Serial.readBytesUntil(';', incomingBytes, sizeof(incomingBytes) / sizeof(char) - 1 );
      if (incomingBytes[0] == '+') {
        triggerMode = Positive;
        triggerLevel = atol(&incomingBytes[1]);
        triggerPrevSample = analogRead(sensorPin);
        ScopeMode = WaitTrigger;
      } else if (incomingBytes[0] == '-') {
        triggerMode = Negative;
        triggerLevel = atoi(&incomingBytes[1]);        
        triggerPrevSample = analogRead(sensorPin);        
        ScopeMode = WaitTrigger;
      } else {
        triggerMode = FreeRunning;
        ScopeMode = Idle;
        Serial.println("$f#:");
      }
    }
  }
  int i;
  
  if (triggerMode == Positive) {
    int triggerSample = analogRead(sensorPin);
    if ((triggerPrevSample <  triggerLevel) && (triggerSample >=  triggerLevel)) {
      Serial.print("$t");
      if (ScopeMode == WaitTrigger) {
        ScopeMode = Measure;
      }
      triggerMode = FreeRunning;
    }
    triggerPrevSample = triggerSample;
  } else if (triggerMode == Negative) {
    int triggerSample = analogRead(sensorPin);
    if ((triggerPrevSample >=  triggerLevel) && (triggerSample <  triggerLevel)) {
      Serial.print("$t");
      if (ScopeMode == WaitTrigger) {
        ScopeMode = Measure;
      }
      triggerMode = FreeRunning;
    }
    triggerPrevSample = triggerSample;
  }
  
  if (ScopeMode == Measure) { 
    for (i=0;i<DisplayXRange;i++) {
      // read the value from the sensor:    
      Values[i] = analogRead(sensorPin);
      if (inputDelay != 0) {
        if (inputDelay < InputDelayRange) {
          delayMicroseconds(delayus);
        } else {
          delay(delayms);
        }
      }
    }

    Serial.print("s");
    Serial.print("#");
    for (i=0;i<DisplayXRange;i++) {
      Serial.print(Values[i], DEC);
      if (i < DisplayXRange-1) Serial.print(";");
    }
    Serial.println(":");
    ScopeMode = Idle;
  }
  
  if (LedState == 0) {
    digitalWrite(ledPin, HIGH);
    LedState++;
  } else if (LedState == 20000) {
    digitalWrite(ledPin, LOW);
    LedState++;
  } else if (LedState == 30000) {
    LedState = 0;
  } else {
    LedState++;
  }
}
