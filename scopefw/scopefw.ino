const long boudRate = 115200;

const int DisplayXRange = 400;
const char StartCmd = 's';
const char DelayCmd = 'd';
const char TriggerCmd = 't';

int Values[DisplayXRange];
unsigned int CollectingCounter = 0;
unsigned int Decimation = 1;
unsigned int DecimationCounter = 0;

enum eTriggerMode { FreeRunning, Positive, Negative};

#define ADCModeIdle            0
#define ADCModeCollecting      1
#define ADCModeColectionDone   2
#define ADCModeWaitingTrigger  3
#define ADCModePreTrigger      4
unsigned short int ADCMode = ADCModeIdle;

const int InputDelayRange = 10000;
int LedState = 0;
eTriggerMode triggerMode = FreeRunning;
int triggerLevel = 0;

void setup() {
  Serial.begin(boudRate);		// Initialize serial communications with the PC
  while (!Serial);
  Serial.setTimeout(1000);
  DDRB = (1 << DDB5); // pin 13 (PD5) в режиме OUTPUT
  DIDR0 = 0x3F; // отключаем цифровые входы на A0-A5 
  ADMUX = 0; // измеряем на ADC0, используем внешнее опорное напр.= 2.495(В)
  ADCSRA = 0xAC; // включаем АЦП, разрешаем прерывания, делитель = 16
  ADCSRB = 0x40; // включаем АЦП каналы MUX, режим скользящей выборки
  bitWrite(ADCSRA, 6, 1); // Запускаем преобразование установкой бита 6 (=ADSC) в ADCSRA
  sei(); // разрешаем прерывания
}

void loop() {
  while (Serial.available() > 0) {
    char inByte = Serial.read();
    if (inByte == StartCmd) {
      cli(); //запрещаем прерывания
      DecimationCounter = 0;
      CollectingCounter = 0;
      ADCMode = ADCModeCollecting;
      sei(); // разрешаем прерывания      
      Serial.print("$");
    } else if (inByte == DelayCmd) {
      char incomingBytes[10];
      memset(incomingBytes, 0, sizeof(incomingBytes));
      Serial.readBytesUntil(';', incomingBytes, sizeof(incomingBytes) / sizeof(char) - 1);
      long inputDelay = atol(incomingBytes);
      inputDelay = (inputDelay > 10000 ? 10000 : inputDelay);
      Serial.print("$d#");
      Serial.print(inputDelay, DEC);
      Serial.println(";:");
      
      cli(); //запрещаем прерывания
      Decimation = inputDelay;
      sei(); // разрешаем прерывания
      
    } else if (inByte == TriggerCmd) {
      char incomingBytes[10];
      memset(incomingBytes, 0, sizeof(incomingBytes));
      Serial.readBytesUntil(';', incomingBytes, sizeof(incomingBytes) / sizeof(char) - 1 );
      if (incomingBytes[0] == '+') {
        long lbuf = atol(&incomingBytes[1]);
        cli(); //запрещаем прерывания        
        triggerMode = Positive;
        triggerLevel = lbuf;
        DecimationCounter = 0;
        CollectingCounter = 0;
        ADCMode = ADCModeWaitingTrigger;
        sei(); // разрешаем прерывания        
        Serial.print("$t");
      } else if (incomingBytes[0] == '-') {
        long lbuf = atol(&incomingBytes[1]);        
        cli(); //запрещаем прерывания        
        triggerMode = Negative;
        triggerLevel = lbuf;
        DecimationCounter = 0;
        CollectingCounter = 0;
        ADCMode = ADCModeWaitingTrigger;
        sei(); // разрешаем прерывания
        Serial.print("$t");
      } else {
        Serial.println("$f#:");
      }
    }
  }
  int i;
  
  if (ADCMode == ADCModeColectionDone) { 
    Serial.print("s");
    Serial.print("#");
    for (i=0;i<DisplayXRange;i++) {
      Serial.print(Values[i], DEC);
      if (i < DisplayXRange-1) Serial.print(";");
    }
    Serial.println(":");
    ADCMode = ADCModeIdle;
  }
}

/*** Процедура обработки прерывания АЦП ***/
ISR(ADC_vect) 
{
  int analogValue;
  
  analogValue = ADCL; // сохраняем младший байт результата АЦП
  analogValue += ADCH << 8; // сохраняем старший байт АЦП
  
  if (LedState == 0) {
    PORTB = (1<<PB5); // пин 13 переводим в состояние HIGH
    LedState++;
  } else if (LedState == 20000) {
    PORTB = 0; // пин 13 переводим в состояние LOW
    LedState++;
  } else if (LedState == 30000) {
    LedState = 0;
  } else {
    LedState++;
  }
  
  if ((ADCMode == ADCModeIdle) || (ADCMode == ADCModeColectionDone)) {
    return;
  }
  
  if (ADCMode == ADCModeWaitingTrigger) {
    if (triggerMode == Positive) {
      if (analogValue <  triggerLevel) {
        ADCMode = ADCModePreTrigger;
      }
    } else if (triggerMode == Negative) {
      if (analogValue >  triggerLevel) {
        ADCMode = ADCModePreTrigger;        
      }
    }
  }
  
  if (ADCMode == ADCModePreTrigger) {
    if (triggerMode == Positive) {
      if (analogValue >=  triggerLevel) {
        ADCMode = ADCModeCollecting;
      }
    } else if (triggerMode == Negative) {
      if (analogValue <=  triggerLevel) {
        ADCMode = ADCModeCollecting;
      }
    }
  }
  
  if (ADCMode == ADCModeCollecting) {
    DecimationCounter++;
    if (DecimationCounter > Decimation) {
      DecimationCounter = 0;
    }
    if (DecimationCounter != 0) {
      return;
    }
    
    Values[CollectingCounter++] = analogValue;
    if (CollectingCounter >= sizeof(Values)/sizeof(Values[0])) {
      CollectingCounter = 0;
      ADCMode = ADCModeColectionDone;
    }
  }
}
