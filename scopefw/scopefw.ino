const long boudRate = 115200;
const int DisplayXRange = 400;
const char StartCmd = 's';
const char DelayCmd = 'd';
const char TriggerCmd = 't';

int ValuesMin[DisplayXRange];
int ValuesMax[DisplayXRange];
unsigned int Decimation = 1;

#define TriggerModeFreeRunning       0
#define TriggerModePositive          1
#define TriggerModeNegative          2

#define FrequencyMeasureIdle         0
#define FrequencyMeasure             1
#define FrequencyMeasureBelow        2

#define ADCModeIdle                  0
#define ADCModeCollecting            1
#define ADCModeColectionDone         2
#define ADCModeWaitingTrigger        3
#define ADCModePreTrigger            4

unsigned short int ADCMode = ADCModeIdle;

const int InputDelayRange = 10000;
int LedState = 0;
unsigned short int triggerMode = TriggerModeFreeRunning;
unsigned short int FrMode = FrequencyMeasureIdle;
int triggerLevel = 512;
unsigned long ADCSamplesCount;
unsigned long ADCChangeCount;
unsigned long CollectingCounter;
unsigned long DecimationCounter;

int maximum;
int minimum;

void setup() {
  Serial.begin(boudRate);		// Initialize serial communications with the PC
  while (!Serial);
  Serial.setTimeout(1000);
  InitValues();
  DDRB = (1 << DDB5); // pin 13 (PD5) в режиме OUTPUT
  DIDR0 = 0x3F; // отключаем цифровые входы на A0-A5 
  ADMUX = 0; // измеряем на ADC0, используем внешнее опорное напр.= 2.495(В)
  ADCSRA = 0xAC; // включаем АЦП, разрешаем прерывания, делитель = 16
  ADCSRB = 0x40; // включаем АЦП каналы MUX, режим скользящей выборки
  bitWrite(ADCSRA, 6, 1); // Запускаем преобразование установкой бита 6 (=ADSC) в ADCSRA
  sei(); // разрешаем прерывания
}

void InitValues(void)
{
  ADCSamplesCount = 0;
  ADCChangeCount = 0;
  CollectingCounter = 0;
  DecimationCounter = 0;
  minimum = 1023;
  maximum = 0;
}

void loop() {
  int i;
  while (Serial.available() > 0) {
    char inByte = Serial.read();
    if (inByte == StartCmd) {
      cli(); //запрещаем прерывания
      InitValues();
      FrMode = FrequencyMeasure;
      ADCMode = ADCModeCollecting;
      sei(); // разрешаем прерывания      
      Serial.print("$");
    }else if (inByte == DelayCmd) {
      char incomingBytes[10];
      memset(incomingBytes, 0, sizeof(incomingBytes));
      Serial.readBytesUntil(';', incomingBytes, sizeof(incomingBytes) / sizeof(char) - 1);
      long inputDecimation = atol(incomingBytes);
      inputDecimation = (inputDecimation > 10000 ? 10000 : inputDecimation);
      Serial.print("$d#");
      Serial.print(inputDecimation, DEC);
      Serial.println(";:");
      cli(); //запрещаем прерывания
      Decimation = inputDecimation;
      sei(); // разрешаем прерывания
    } else if (inByte == TriggerCmd) {
      char incomingBytes[10];
      memset(incomingBytes, 0, sizeof(incomingBytes));
      Serial.readBytesUntil(';', incomingBytes, sizeof(incomingBytes) / sizeof(char) - 1 );
      if (incomingBytes[0] == '+') {
        long lbuf = atol(&incomingBytes[1]);
        cli(); //запрещаем прерывания        
        triggerMode = TriggerModePositive;
        triggerLevel = lbuf;
        InitValues();
        FrMode = FrequencyMeasure;        
        ADCMode = ADCModeWaitingTrigger;
        sei(); // разрешаем прерывания        
        Serial.print("$t");
      } else if (incomingBytes[0] == '-') {
        long lbuf = atol(&incomingBytes[1]);        
        cli(); //запрещаем прерывания
        triggerMode = TriggerModeNegative;
        triggerLevel = lbuf;
        InitValues();        
        FrMode = FrequencyMeasure;
        ADCMode = ADCModeWaitingTrigger;
        sei(); // разрешаем прерывания
        Serial.print("$t");
      } else {
        Serial.println("$r#:");
      }
    }
  }
  if (ADCMode == ADCModeColectionDone) {
    cli(); //запрещаем прерывания
    ADCMode = ADCModeIdle;
    FrMode = FrequencyMeasureIdle;
    sei(); // разрешаем прерывания    
    Serial.print("s");
    Serial.print("#");
    Serial.print(ADCChangeCount, DEC);
    Serial.print(";");
    Serial.print(ADCSamplesCount, DEC);
    for (i=0;i<DisplayXRange;i++) {
      Serial.print(";");
      Serial.print(ValuesMax[i], DEC);
      Serial.print(";");      
      Serial.print(ValuesMin[i], DEC);      
    }
    Serial.println(":");
  }
}

/*** Процедура обработки прерывания АЦП ***/
ISR(ADC_vect)
{
  int analogValue;
  
  PORTB = (1<<PB5); // пин 13 переводим в состояние HIGH
  
  analogValue = ADCL; // сохраняем младший байт результата АЦП
  analogValue += ADCH << 8; // сохраняем старший байт АЦП

  if (ADCMode == ADCModeIdle) {
    PORTB = 0; // пин 13 переводим в состояние LOW    
    return;
  }
  
  if (ADCMode == ADCModeWaitingTrigger) {
    if (triggerMode == TriggerModePositive) {
      if (analogValue <  triggerLevel) {
        ADCMode = ADCModePreTrigger;
      }
    } else if (triggerMode == TriggerModeNegative) {
      if (analogValue >  triggerLevel) {
        ADCMode = ADCModePreTrigger;        
      }
    }
  }
  
  if (ADCMode == ADCModePreTrigger) {
    if (triggerMode == TriggerModePositive) {
      if (analogValue >=  triggerLevel) {
        ADCMode = ADCModeCollecting;
      }
    } else if (triggerMode == TriggerModeNegative) {
      if (analogValue <=  triggerLevel) {
        ADCMode = ADCModeCollecting;
      }
    }
  }
  
  if (ADCMode == ADCModeCollecting) {
    
    if (FrMode == FrequencyMeasure) {
      ADCSamplesCount++;
      if (analogValue < triggerLevel) {
        FrMode = FrequencyMeasureBelow;
        ADCChangeCount++;      
      }
    } else if (FrMode == FrequencyMeasureBelow) {
      ADCSamplesCount++;
      if (analogValue >= triggerLevel) {
        FrMode = FrequencyMeasure;
        ADCChangeCount++;
      }
    }
    
    if (maximum < analogValue) {
      maximum = analogValue;
    }
    if (minimum > analogValue) {
      minimum = analogValue;
    }
    
    DecimationCounter++;
    if (DecimationCounter > Decimation) {
      DecimationCounter = 0;
    }
    if (DecimationCounter != 0) {
      PORTB = 0; // пин 13 переводим в состояние LOW    
      return;
    }
    
    ValuesMax[CollectingCounter] = maximum;
    ValuesMin[CollectingCounter] = minimum;
    
    maximum = 0;
    minimum = 1023;
    
    CollectingCounter++;
    if (CollectingCounter >= sizeof(ValuesMax)/sizeof(ValuesMax[0])) {
      CollectingCounter = 0;
      ADCMode = ADCModeColectionDone;
    }
  }
  PORTB = 0; // пин 13 переводим в состояние LOW    
}
