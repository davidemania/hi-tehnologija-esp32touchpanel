#include "misc/lv_color.h"
#include "esp32-hal.h"
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

// Using LVGL with Arduino requires some extra steps:
// Be sure to read the docs here: https://docs.lvgl.io/master/get-started/platforms/arduino.html  */

#include "Display.h"

#define COLOR_BG_BUTTON_MMR 0x18b1db
#define COLOR_BG_BUTTON_DND 0xdb1845
#define COLOR_BG_BUTTON_INACTIVE 0x808080
#define COLOR_BG_SCREEN 0x000000

#define BUTTON_BORDER_COLOR 0xfffff

#define BUTTON_RADIUS 20

#define BUZZER_PIN EXIO_PIN8

// *********************************************************************************
// If you embed the image as a C array from LVGL's image converter:

LV_IMG_DECLARE(splash_480x640); // 480x640, RGB565, no alpha

// *********************************************************************************

static const lv_font_t *font_large;
static const lv_font_t *font_normal;

static lv_style_t styleButtonMmr;
static lv_style_t styleButtonDnd;
static lv_style_t styleScreen;
static lv_style_t styleLabelMmr;
static lv_style_t styleLabelDnd;

static lv_obj_t *button_clean_my_room;
lv_obj_t *button_do_not_disturb;
static lv_obj_t *Backlight_slider;
static lv_obj_t * label_dnd;
static lv_obj_t * label_mmr;

static lv_timer_t * auto_step_timer;

static bool doNotDisturbIsActive = false;
static bool makeMyRoomIsActive = false;

void show_splash(void);

static void splash_done_cb(lv_timer_t *t);
static void backlight_fade(uint8_t from, uint8_t to, uint16_t ms);
static void Backlight_adjustment_event_cb(lv_event_t * e);

static void setStyleDndOn();
static void setStyleMmrOn();
static void setStyleOff();

// *********************************************************************************

static void button_clean_my_room_event_cb(lv_event_t * e)
{
  lv_event_code_t code = lv_event_get_code(e);
  // lv_obj_t * btn = lv_event_get_target(e);
  if(code == LV_EVENT_CLICKED)
  {
    Set_EXIO(BUZZER_PIN, HIGH);
    delay(100);
    Set_EXIO(BUZZER_PIN, LOW);
    makeMyRoomIsActive = !makeMyRoomIsActive;

    setStyleOff();
    if(makeMyRoomIsActive)
    {
      doNotDisturbIsActive = false;
      setStyleMmrOn();
    }

    button_clicked(makeMyRoomIsActive, doNotDisturbIsActive);
  }
}

// *********************************************************************************

static void button_do_not_disturb_event_cb(lv_event_t * e)
{
  lv_event_code_t code = lv_event_get_code(e);
  // lv_obj_t * btn = lv_event_get_target(e);
  if(code == LV_EVENT_CLICKED)
  {
    Set_EXIO(BUZZER_PIN, HIGH);
    delay(100);
    Set_EXIO(BUZZER_PIN, LOW);
    doNotDisturbIsActive = !doNotDisturbIsActive;

    setStyleOff();
    if(doNotDisturbIsActive)
    {
      makeMyRoomIsActive = false;
      setStyleDndOn();
    }

    button_clicked(makeMyRoomIsActive, doNotDisturbIsActive);
  }
}

// *********************************************************************************

void initStyles()
{
  lv_style_init(&styleButtonDnd);
  lv_style_init(&styleButtonMmr);
  lv_style_init(&styleLabelDnd);
  lv_style_init(&styleLabelMmr);
  lv_style_init(&styleScreen);

  lv_style_set_bg_color(&styleScreen, lv_color_hex(COLOR_BG_SCREEN));
  lv_obj_add_style(lv_scr_act(), &styleScreen, LV_STATE_DEFAULT);

  lv_style_set_border_width(&styleButtonMmr, 2);
  // lv_style_set_border_opa(&styleButtonMmr, LV_OPA_20);
  lv_style_set_radius(&styleButtonMmr, BUTTON_RADIUS);

  lv_style_set_border_width(&styleButtonDnd, 2);
  // lv_style_set_border_opa(&styleButtonDnd, LV_OPA_20);
  lv_style_set_radius(&styleButtonDnd, BUTTON_RADIUS);

  lv_style_set_text_font(&styleLabelDnd, font_large);
  lv_style_set_text_font(&styleLabelMmr, font_large);

  lv_obj_add_style(button_do_not_disturb, &styleButtonDnd, LV_STATE_DEFAULT);
  lv_obj_add_style(button_clean_my_room, &styleButtonMmr, LV_STATE_DEFAULT);

  lv_obj_add_style(label_dnd, &styleLabelDnd, LV_STATE_DEFAULT);
  lv_obj_add_style(label_mmr, &styleLabelMmr, LV_STATE_DEFAULT);

  setStyleOff();
}

// *********************************************************************************

void setStyleOff()
{
  // Do not disturb button
  lv_style_set_bg_color(&styleButtonDnd, lv_color_hex(COLOR_BG_BUTTON_INACTIVE));
  lv_style_set_border_color(&styleButtonDnd, lv_color_hex(BUTTON_BORDER_COLOR));
  lv_style_set_border_width(&styleButtonDnd, 2);
  // lv_style_set_border_opa(&styleButtonDnd, LV_OPA_20);
  lv_style_set_radius(&styleButtonDnd, BUTTON_RADIUS);
  
  // Clean room button
  lv_style_set_bg_color(&styleButtonMmr, lv_color_hex(COLOR_BG_BUTTON_INACTIVE));
  lv_style_set_border_color(&styleButtonMmr, lv_color_hex(BUTTON_BORDER_COLOR));
  lv_style_set_border_width(&styleButtonMmr, 2);
  // lv_style_set_border_opa(&styleButtonMmr, LV_OPA_20);
  lv_style_set_radius(&styleButtonMmr, BUTTON_RADIUS);
  
  // labels
  lv_style_set_text_color(&styleLabelDnd, lv_color_black());
  lv_style_set_text_color(&styleLabelMmr, lv_color_black());

  // Tell LVGL the attached styles changed
  lv_obj_report_style_change(&styleButtonDnd);
  lv_obj_report_style_change(&styleButtonMmr);
  lv_obj_report_style_change(&styleLabelDnd);
  lv_obj_report_style_change(&styleLabelMmr);
}

// *********************************************************************************

void setStyleDndOn()
{
  lv_style_set_bg_color(&styleButtonDnd, lv_color_hex(COLOR_BG_BUTTON_DND));
  lv_style_set_border_color(&styleButtonDnd, lv_color_hex(BUTTON_BORDER_COLOR));
  
  lv_obj_add_style(button_do_not_disturb, &styleButtonDnd, LV_STATE_DEFAULT);
  
  lv_style_set_text_color(&styleLabelDnd, lv_color_white());

  lv_obj_report_style_change(&styleButtonDnd);
  lv_obj_report_style_change(&styleLabelDnd);
}

// *********************************************************************************

void setStyleMmrOn()
{  
  lv_style_set_bg_color(&styleButtonMmr, lv_color_hex(COLOR_BG_BUTTON_MMR));
  lv_style_set_border_color(&styleButtonMmr, lv_color_hex(BUTTON_BORDER_COLOR));

  lv_style_set_text_color(&styleLabelMmr, lv_color_white());

  lv_obj_report_style_change(&styleButtonMmr);
  lv_obj_report_style_change(&styleLabelMmr);
}

// *********************************************************************************

void buildUiPanel()
{
  font_large = &lv_font_montserrat_36; // LV_FONT_DEFAULT;                             
  font_normal = &lv_font_montserrat_24; // LV_FONT_DEFAULT;                         

  button_clean_my_room = lv_btn_create(lv_scr_act());
  lv_obj_set_pos(button_clean_my_room, 20, 20);
  lv_obj_set_size(button_clean_my_room, 440, 240);
  lv_obj_add_event_cb(button_clean_my_room, button_clean_my_room_event_cb, LV_EVENT_ALL, NULL);

  label_mmr = lv_label_create(button_clean_my_room);
  lv_label_set_text(label_mmr, "Please clean Room");
  lv_obj_center(label_mmr);

  button_do_not_disturb = lv_btn_create(lv_scr_act());
  lv_obj_set_pos(button_do_not_disturb, 20, 310);
  lv_obj_set_size(button_do_not_disturb, 440, 240);
  lv_obj_add_event_cb(button_do_not_disturb, button_do_not_disturb_event_cb, LV_EVENT_ALL, NULL);

  label_dnd = lv_label_create(button_do_not_disturb);
  lv_label_set_text(label_dnd, "Do not Disturb");
  lv_obj_center(label_dnd);
  
  Backlight_slider = lv_slider_create(lv_scr_act());
  lv_obj_set_pos(Backlight_slider, 20, 580);
  lv_obj_add_flag(Backlight_slider, LV_OBJ_FLAG_CLICKABLE);    
  lv_obj_set_size(Backlight_slider, 290, 45);              
  // lv_obj_set_style_radius(Backlight_slider, 3, LV_PART_KNOB);               // Adjust the value for more or less rounding                                            
  // // lv_obj_set_style_bg_opa(Backlight_slider, LV_OPA_TRANSP, LV_PART_KNOB);                               
  // // lv_obj_set_style_pad_all(Backlight_slider, 0, LV_PART_KNOB);                                            
  lv_obj_set_style_bg_color(Backlight_slider, lv_color_hex(0xAAAAAA), LV_PART_KNOB);               
  lv_obj_set_style_bg_color(Backlight_slider, lv_color_hex(0xFFFFFF), LV_PART_INDICATOR);             
  lv_obj_set_style_outline_width(Backlight_slider, 2, LV_PART_INDICATOR);  
  lv_obj_set_style_outline_color(Backlight_slider, lv_color_hex(0xD3D3D3), LV_PART_INDICATOR);      
  lv_slider_set_range(Backlight_slider, 5, Backlight_MAX);              
  lv_slider_set_value(Backlight_slider, LCD_Backlight, LV_ANIM_ON);  
  lv_obj_add_event_cb(Backlight_slider, Backlight_adjustment_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

  initStyles();
}

// *********************************************************************************
// Never called
// *********************************************************************************

void close_ui_panel()
{
  lv_obj_clean(lv_scr_act());

  // lv_style_reset(&style_text_muted);
  // lv_style_reset(&style_title);
  // lv_style_reset(&style_icon);
  // lv_style_reset(&style_bullet);
}

// *********************************************************************************

void lv_timer_tick(lv_timer_t * t)
{
  // char buf[100]; 
  
  // snprintf(buf, sizeof(buf), "%d MB\r\n", 0);

  // lv_slider_set_value(Backlight_slider, LCD_Backlight, LV_ANIM_ON); 
  // Set_Backlight(LCD_Backlight);
}

// *********************************************************************************

void Backlight_adjustment_event_cb(lv_event_t * e)
{
  uint8_t Backlight = lv_slider_get_value(lv_event_get_target(e));

  if (Backlight >= 0 && Backlight <= Backlight_MAX) 
  {
    lv_slider_set_value(Backlight_slider, Backlight, LV_ANIM_ON); 
    LCD_Backlight = Backlight;
    Set_Backlight(Backlight);

    // Serial.printf("Backlight changed: %d\n", Backlight);
  }
}

// *********************************************************************************

void splash_done_cb(lv_timer_t *t)
{
  lv_obj_t *splash = (lv_obj_t *)t->user_data;
  lv_timer_del(t);            // one-shot
  lv_obj_del(splash);         // free splash screen
  buildUiPanel();             // go to your normal UI
}

// *********************************************************************************

void backlight_fade(uint8_t from, uint8_t to, uint16_t ms)
{
  uint32_t t0 = millis();
  while (millis() - t0 < ms)
  {
    uint32_t t = millis() - t0;
    uint8_t v = from + (int)( (to - from) * (float)t / ms );
    Set_Backlight(v);
    lv_timer_handler();
    delay(5);
  }
}

// *********************************************************************************

void show_splash(void)
{
  // // bare screen with solid background
  // lv_obj_t *splash = lv_obj_create(NULL);
  // lv_obj_remove_style_all(splash);
  // lv_obj_set_style_bg_color(splash, lv_color_black(), 0);
  // lv_obj_set_style_bg_opa(splash, LV_OPA_COVER, 0);

  // // full-screen image
  // lv_obj_t *img = lv_img_create(splash);
  // lv_img_set_src(img, &splash_480x640);
  // lv_obj_center(img);

  // // display it
  // lv_scr_load(splash);

  // // keep ~2s, then switch to your main UI
  // lv_timer_create(splash_done_cb, 2000, splash);
}

// *********************************************************************************
// *********************************************************************************
