// ********************************************************************************* //
// *   _    _ _     _______               _     _____  _           _               * //
// *  | |  | (_)   |__   __|             | |   |  __ \(_)         | |              * //
// *  | |__| |_ ______| | ___  _   _  ___| |__ | |  | |_ ___ _ __ | | __ _ _   _   * //
// *  |  __  | |______| |/ _ \| | | |/ __| '_ \| |  | | / __| '_ \| |/ _` | | | |  * //
// *  | |  | | |      | | (_) | |_| | (__| | | | |__| | \__ \ |_) | | (_| | |_| |  * //
// *  |_|  |_|_|      |_|\___/ \__,_|\___|_| |_|_____/|_|___/ .__/|_|\__,_|\__, |  * //
// *                                                        | |             __/ |  * //
// *                                                        |_|            |___/   * //
// *                                                                               * //
// * Touch controller for Hi-HLM Smart Door Project                                * //
// * Based on Waveshare ESP32-S3-Touch-LCD-2.8B Module                             * //
// * By Davide Manià 2025, (c) Studio Cogitamus & Hi-Tehnologija doo               * //
// * For info and support contact software@cogitamus.it                            * //
// *                                                                               * //
// ********************************************************************************* //

#include "I2C_Driver.h"
#include "Display.h"

#include "driver/uart.h"
#include "driver/gpio.h"

static void claim_pulse_pin_43()
{
  // Make sure UART0 is not using IO43
  Serial0.end();  // if it was started anywhere
  uart_set_pin(UART_NUM_0, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE); // detach pins

  // Reset pad/matrix back to plain GPIO
  gpio_reset_pin((gpio_num_t)43);
  gpio_set_pull_mode((gpio_num_t)43, GPIO_FLOATING);

  // Now configure and drive it
  pinMode(43, OUTPUT);          // not OPEN_DRAIN
  digitalWrite(43, LOW);        // known state
}

// *********************************************************************************

enum PulseDuration {
    PLEASE_MAKE_UP_ROOM = 400,
    DO_NOT_DISTURB = 300,
    IDLE = 200,
    COUNT
} pulseDuration;

// *********************************************************************************

void button_clicked(bool mmr, bool dnd)
{
  Serial.printf("Button clicked mmr %d dnd %d", mmr, dnd);
  Serial.println();

  if(mmr)
  {
    pulseDuration = PLEASE_MAKE_UP_ROOM;
  }
  else if(dnd)
  {
    pulseDuration = DO_NOT_DISTURB;
  }
  else
  {
    pulseDuration = IDLE;
  }
}

// *********************************************************************************

const int PULSE_GPIO_PIN = 43; // reassigned from TXD opn this board

void setup()
{
  Serial.begin(115200);
  
  I2C_Init();
  TCA9554PWR_Init(0x00);
  Set_EXIO(EXIO_PIN8,Low);
  Backlight_Init();
  
  // pinMode(PULSE_GPIO_PIN, OUTPUT);
  // digitalWrite(PULSE_GPIO_PIN, LOW);
  
  LCD_Init();
  Lvgl_Init();

  claim_pulse_pin_43();
  pulseDuration = IDLE;

  show_splash();

  Serial.println("Initialization done!");
}

// *********************************************************************************

const int PULSE_PERIOD_TIME_MS = 1000;
unsigned long lastPulseSetTime = millis();

void loop()
{
  const int elapsed = millis() - lastPulseSetTime;

  if(elapsed > PULSE_PERIOD_TIME_MS)
  {
    digitalWrite(PULSE_GPIO_PIN, HIGH);
    lastPulseSetTime = millis();
    Serial.printf("Elapsed: %d ", elapsed); Serial.println("High");
  }
  else if(elapsed > pulseDuration)
  {
    if(digitalRead(PULSE_GPIO_PIN) == HIGH)
    {
      digitalWrite(PULSE_GPIO_PIN, LOW);
      Serial.printf("Elapsed: %d ", elapsed); Serial.println("Low");
    }
  }

  Lvgl_Loop();
  delay(1);
}

// *********************************************************************************
// *********************************************************************************
