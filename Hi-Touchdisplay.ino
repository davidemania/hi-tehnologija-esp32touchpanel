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

#include "LVGL_Driver.h"
#include "Display_ST7701.h"

// *********************************************************************************
// If you embed the image as a C array from LVGL's image converter:

LV_IMG_DECLARE(splash_480x640); // 480x640, RGB565, no alpha

// *********************************************************************************

static lv_obj_t * tv;
lv_style_t style_text_muted;
lv_style_t style_title;
static lv_style_t style_icon;
static lv_style_t style_bullet;

static const lv_font_t * font_large;
static const lv_font_t * font_normal;

static lv_timer_t * auto_step_timer;

void show_splash(void);
static void splash_done_cb(lv_timer_t *t);
static void backlight_fade(uint8_t from, uint8_t to, uint16_t ms);

lv_obj_t * SD_Size;
lv_obj_t * Backlight_slider;

// *********************************************************************************

void buildUiPanel(void)
{
  font_large = &lv_font_montserrat_28; // LV_FONT_DEFAULT;                             
  font_normal = &lv_font_montserrat_18; // LV_FONT_DEFAULT;                         
  
  lv_coord_t tab_h;
  
  tab_h = 45;

  lv_style_init(&style_text_muted);
  lv_style_set_text_opa(&style_text_muted, LV_OPA_90);

  lv_style_init(&style_title);
  lv_style_set_text_font(&style_title, font_large);

  lv_style_init(&style_icon);
  lv_style_set_text_color(&style_icon, lv_theme_get_color_primary(NULL));
  lv_style_set_text_font(&style_icon, font_large);

  lv_style_init(&style_bullet);
  lv_style_set_border_width(&style_bullet, 0);
  lv_style_set_radius(&style_bullet, LV_RADIUS_CIRCLE);

  tv = lv_tabview_create(lv_scr_act(), LV_DIR_TOP, tab_h);

  lv_obj_set_style_text_font(lv_scr_act(), font_normal, 0);

  lv_obj_t * t1 = lv_tabview_add_tab(tv, "DEMO");
  
  Onboard_create(t1);
}

// *********************************************************************************

static void led_event_cb(lv_event_t *e)
{
    lv_obj_t *led = (lv_obj_t *)lv_event_get_user_data(e);
    lv_obj_t *sw = lv_event_get_target(e); 
 
    if (lv_obj_get_state(sw) & LV_STATE_CHECKED)
    {
      lv_led_on(led);
      Set_EXIO(EXIO_PIN8,High);
    } 
    else
    {
      lv_led_off(led);
      Set_EXIO(EXIO_PIN8,Low);
    }
}

// *********************************************************************************

static void Buzzer_create(lv_obj_t * parent)
{
  lv_obj_t *label = lv_label_create(parent);
  lv_label_set_text(label, "The buzzer tes");
  lv_obj_set_size(label, LV_PCT(30), LV_PCT(5));
  lv_obj_align(label, LV_ALIGN_CENTER, 0, -60);
  lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);  


  lv_obj_t *led = lv_led_create(parent);
  lv_obj_set_size(led, 50, 50);
  lv_obj_align(led, LV_ALIGN_CENTER, -60, 0);
  lv_led_off(led);

  lv_obj_t *sw = lv_switch_create(parent);
  lv_obj_align(sw, LV_ALIGN_CENTER, 60, 0);
  lv_obj_add_event_cb(sw, led_event_cb, LV_EVENT_ALL, led);
}

// *********************************************************************************

void close_ui_panel(void)
{
  lv_obj_clean(lv_scr_act());

  lv_style_reset(&style_text_muted);
  lv_style_reset(&style_title);
  lv_style_reset(&style_icon);
  lv_style_reset(&style_bullet);
}

// *********************************************************************************

static void Onboard_create(lv_obj_t * parent)
{
  /*Create a panel*/
  lv_obj_t * panel1 = lv_obj_create(parent);
  lv_obj_set_height(panel1, LV_SIZE_CONTENT);

  lv_obj_t * panel1_title = lv_label_create(panel1);
  lv_label_set_text(panel1_title, "PANEL 123");
  lv_obj_add_style(panel1_title, &style_title, 0);

  lv_obj_t * SD_label = lv_label_create(panel1);
  lv_label_set_text(SD_label, "SD Card");
  lv_obj_add_style(SD_label, &style_text_muted, 0);

  SD_Size = lv_textarea_create(panel1);
  lv_textarea_set_one_line(SD_Size, true);
  lv_textarea_set_placeholder_text(SD_Size, "SD Size");
  lv_obj_add_event_cb(SD_Size, ta_event_cb, LV_EVENT_ALL, NULL);

  lv_obj_t * Backlight_label = lv_label_create(panel1);
  lv_label_set_text(Backlight_label, "Backlight brightness");
  lv_obj_add_style(Backlight_label, &style_text_muted, 0);

  Backlight_slider = lv_slider_create(panel1);                                 
  lv_obj_add_flag(Backlight_slider, LV_OBJ_FLAG_CLICKABLE);    
  lv_obj_set_size(Backlight_slider, 200, 35);              
  lv_obj_set_style_radius(Backlight_slider, 3, LV_PART_KNOB);               // Adjust the value for more or less rounding                                            
  lv_obj_set_style_bg_opa(Backlight_slider, LV_OPA_TRANSP, LV_PART_KNOB);                               
  // lv_obj_set_style_pad_all(Backlight_slider, 0, LV_PART_KNOB);                                            
  lv_obj_set_style_bg_color(Backlight_slider, lv_color_hex(0xAAAAAA), LV_PART_KNOB);               
  lv_obj_set_style_bg_color(Backlight_slider, lv_color_hex(0xFFFFFF), LV_PART_INDICATOR);             
  lv_obj_set_style_outline_width(Backlight_slider, 2, LV_PART_INDICATOR);  
  lv_obj_set_style_outline_color(Backlight_slider, lv_color_hex(0xD3D3D3), LV_PART_INDICATOR);      
  lv_slider_set_range(Backlight_slider, 5, Backlight_MAX);              
  lv_slider_set_value(Backlight_slider, LCD_Backlight, LV_ANIM_ON);  
  lv_obj_add_event_cb(Backlight_slider, Backlight_adjustment_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

  lv_obj_t * panel2 = lv_obj_create(parent);
  lv_obj_set_height(panel2, LV_SIZE_CONTENT);

  lv_obj_t * panel2_title = lv_label_create(panel2);
  lv_label_set_text(panel2_title, "Buzzer control");
  lv_obj_add_style(panel2_title, &style_title, 0);

  lv_obj_t *led = lv_led_create(panel2);
  lv_obj_set_size(led, 50, 50);
  lv_obj_align(led, LV_ALIGN_CENTER, -60, 0);
  lv_led_off(led);

  lv_obj_t *sw = lv_switch_create(panel2);
  lv_obj_set_size(sw, 65, 40);
  lv_obj_align(sw, LV_ALIGN_CENTER, 60, 0);
  lv_obj_add_event_cb(sw, led_event_cb, LV_EVENT_VALUE_CHANGED, led);

  static lv_coord_t grid_main_col_dsc[] = {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
  static lv_coord_t grid_main_row_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

  /*Create the top panel*/
  static lv_coord_t grid_1_col_dsc[] =
  {
    LV_GRID_FR(4),  LV_GRID_FR(1),  LV_GRID_FR(1),  LV_GRID_FR(1), LV_GRID_FR(4), LV_GRID_TEMPLATE_LAST
  };

  static lv_coord_t grid_1_row_dsc[] =
  {
    LV_GRID_CONTENT, /*Name*/
    LV_GRID_CONTENT, /*Description*/
    LV_GRID_CONTENT, /*Email*/
    LV_GRID_CONTENT, /*Email*/
    LV_GRID_TEMPLATE_LAST
  };

  static lv_coord_t grid_2_col_dsc[] =
  {
    LV_GRID_FR(1), LV_GRID_FR(5), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST
  };

  static lv_coord_t grid_2_row_dsc[] =
  {
    LV_GRID_CONTENT,  /*Title*/
    5,                /*Separator*/
    LV_GRID_CONTENT,  /*Box title*/
    40,               /*Box*/
    LV_GRID_TEMPLATE_LAST
  };

  lv_obj_set_grid_dsc_array(parent, grid_main_col_dsc, grid_main_row_dsc);

  lv_obj_set_grid_cell(panel1, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_START, 0, 1);
  lv_obj_set_grid_dsc_array(panel1, grid_2_col_dsc, grid_2_row_dsc);
  lv_obj_set_grid_cell(panel1_title, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 0, 1);
  lv_obj_set_grid_cell(SD_label, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_START, 2, 1);
  lv_obj_set_grid_cell(SD_Size, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_CENTER, 3, 1);
  lv_obj_set_grid_cell(Backlight_label, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_START, 14, 1);
  lv_obj_set_grid_cell(Backlight_slider, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_CENTER, 15, 1);

  lv_obj_set_grid_cell(panel2, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 1, 1);
  lv_obj_set_grid_dsc_array(panel2, grid_1_col_dsc, grid_1_row_dsc);
  lv_obj_set_grid_cell(panel2_title, LV_GRID_ALIGN_CENTER, 2, 1, LV_GRID_ALIGN_CENTER, 0, 1);
  lv_obj_set_grid_cell(led, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 2, 1);
  lv_obj_set_grid_cell(sw, LV_GRID_ALIGN_CENTER, 3, 1, LV_GRID_ALIGN_CENTER, 2, 1);

  auto_step_timer = lv_timer_create(lv_timer_tick, 100, NULL);
}

// *********************************************************************************

void lv_timer_tick(lv_timer_t * t)
{
  char buf[100]; 
  
  snprintf(buf, sizeof(buf), "%d MB\r\n", 0);
  lv_textarea_set_placeholder_text(SD_Size, buf);

  lv_slider_set_value(Backlight_slider, LCD_Backlight, LV_ANIM_ON); 
  Set_Backlight(LCD_Backlight);
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
  }
}

// *********************************************************************************

static void ta_event_cb(lv_event_t * e)
{
}

// *********************************************************************************

void Driver_Init()
{
  
}

// *********************************************************************************

void setup()
{
  I2C_Init();
  TCA9554PWR_Init(0x00);
  Set_EXIO(EXIO_PIN8,Low);
  Backlight_Init();
    
  LCD_Init();
  Lvgl_Init();

  buildUiPanel();
}

// *********************************************************************************

void loop()
{
  Lvgl_Loop();
  delay(5);
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
  // bare screen with solid background
  lv_obj_t *splash = lv_obj_create(NULL);
  lv_obj_remove_style_all(splash);
  lv_obj_set_style_bg_color(splash, lv_color_black(), 0);
  lv_obj_set_style_bg_opa(splash, LV_OPA_COVER, 0);

  // full-screen image
  lv_obj_t *img = lv_img_create(splash);
  lv_img_set_src(img, &splash_480x640);
  lv_obj_center(img);

  // display it
  lv_scr_load(splash);

  // keep ~2s, then switch to your main UI
  lv_timer_create(splash_done_cb, 2000, splash);
}

// *********************************************************************************
// *********************************************************************************
