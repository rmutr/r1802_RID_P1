//------------------------------------------------
// Project     : 
// Board       : ThaiEasyElec's ESPIno
// LED_BUILTIN : Pin 16
//------------------------------------------------

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

#define PIN_LED_PROCESS   16
#define PIN_PULSEIN       13
#define PIN_BUTTON_0      12

#define LCD_ADD_27        0x27
#define LCD_ADD_3F        0x3F

LiquidCrystal_I2C lcd_0(LCD_ADD_27, 16, 2);  

char str_buff[200] = {0};
unsigned long t_old;
int state_ix  = 0;
volatile unsigned long p_count, p_store, p_buff_ix, p_total;
volatile unsigned long p_buff[40] = {0};
volatile int req_reset = 0;

void Interrupt_Service_PulseIn()  { p_count++; }

void Interrupt_Service_Button_0() { req_reset = 1; }

void setup() {
  pinMode(PIN_LED_PROCESS, OUTPUT);
  pinMode(PIN_PULSEIN, INPUT_PULLUP);
  pinMode(PIN_BUTTON_0, INPUT_PULLUP);

  digitalWrite(PIN_LED_PROCESS, LOW);
  digitalWrite(PIN_PULSEIN, HIGH);
  digitalWrite(PIN_BUTTON_0, HIGH);

  attachInterrupt(digitalPinToInterrupt(PIN_PULSEIN)
    , Interrupt_Service_PulseIn, FALLING);  

  attachInterrupt(digitalPinToInterrupt(PIN_BUTTON_0)
    , Interrupt_Service_Button_0, FALLING);  

  Serial.begin(115200); 
  Serial.println("\n\nSystem start..");

  t_old     = 0;
  state_ix  = 0;
  p_store   = 0;
  p_count   = 0;

  p_buff_ix = 0;
  p_total   = 0;

  for (int bix = 0; bix < 40; bix++) {
    p_buff[bix] = 0;
  }

  req_reset = 0;
}

void loop() {
//------------------------------------------------
  p_store = p_count;
  p_count = 0;

//------------------------------------------------
  if (req_reset == 1) { 
    req_reset = 0; 
    if (state_ix > 1) { 
      state_ix = 1; 
    } 
  } 

//------------------------------------------------
  switch (state_ix) {
    default:
    case 0: // Clear
      lcd_0.init();
      lcd_0.backlight();
      lcd_0.setCursor(0, 0);
      lcd_0.print("                ");
      lcd_0.setCursor(0, 1);
      lcd_0.print("                ");

      p_buff_ix = 40;
      state_ix = 3;
      break;

    case 1:
      p_buff_ix = 0;
      p_total = 0;

      for (int bix = 0; bix < 40; bix++) {
        p_buff[bix] = 0;
      }

      state_ix++;
      break;

    case 2:
      if (p_buff_ix < 40) {
        p_buff[p_buff_ix] = p_store;
        p_buff_ix++;

        p_total = 0;
        for (int bix = 0; bix < 40; bix++) {
          p_total += p_buff[bix];
        }
      } else {
        state_ix++;
      }
      break;

    case 3:
      break;
  }

//------------------------------------------------
  if (state_ix > 0) {
    sprintf(str_buff, "Cnt %7lu /%2ds", p_total, p_buff_ix);
    Serial.print(str_buff); Serial.print("   ");
    lcd_0.setCursor(0, 0);
    lcd_0.print(str_buff);

    sprintf(str_buff, "Freq %6lu Hz  ", p_store);
    Serial.println(str_buff);
    lcd_0.setCursor(0, 1);
    lcd_0.print(str_buff);
  }

//------------------------------------------------
  digitalWrite(PIN_LED_PROCESS, LOW);
  while ((micros() - t_old) < 1000000);
  t_old = micros(); 
  digitalWrite(PIN_LED_PROCESS, HIGH);
//------------------------------------------------
}

