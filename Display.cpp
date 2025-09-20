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

#define BUTTON_BORDER_COLOR 0xffffff

#define BUTTON_RADIUS 20

#define BUZZER_PIN EXIO_PIN8

LV_FONT_DECLARE(montserrat_extended);
LV_FONT_DECLARE(noto_chinese);
LV_FONT_DECLARE(noto_russian);
LV_FONT_DECLARE(noto_arabic);
LV_FONT_DECLARE(noto_japanese);
LV_FONT_DECLARE(noto_korean);
  
static const lv_font_t *font_large = &montserrat_extended;
static const lv_font_t *font_normal = &lv_font_montserrat_24;
static const lv_font_t *font_icon = &lv_font_montserrat_32;
static const lv_font_t *font_russian = &noto_russian;
static const lv_font_t *font_arabic = &noto_arabic;
static const lv_font_t *font_japanese = &noto_japanese;
static const lv_font_t *font_korean = &noto_korean;
static const lv_font_t *font_chinese = &noto_chinese;

// Language index
typedef enum {
    LANG_EN = 0, // English
    LANG_IT,     // Italian
    LANG_ZH,     // Chinese (Simplified)
    LANG_FR,     // French
    LANG_RU,     // Russian
    LANG_ES,     // Spanish
    LANG_DE,     // German
    LANG_JA,     // Japanese
    LANG_KO,     // Korean
    LANG_PT,     // Portuguese
    LANG_AR,     // Arabic
    LANG_NL,     // Dutch
    LANG_SL,     // Slovenian
    LANG_COUNT
} Lang;

static const lv_font_t * fonts[LANG_COUNT] = {
  font_large,  // English
  font_large,  // Italian
  font_chinese,  // Chinese (Simplified)
  font_large,  // French
  font_russian,  // Russian
  font_large,  // Spanish
  font_large,  // German
  font_japanese,  // Japanese
  font_korean,  // Korean
  font_large,  // Portuguese
  font_arabic,  // Arabic
  font_large,  // Dutch
  font_large,  // Slovenian
};

static const bool language_is_rtl[LANG_COUNT] = {
  false,  // English
  false,  // Italian
  false,  // Chinese (Simplified)
  false,  // French
  false,  // Russian
  false,  // Spanish
  false,  // German
  false,  // Japanese
  false,  // Korean
  false,  // Portuguese
  true,   // Arabic
  false,  // Dutch
  false,  // Slovenian
};

// Display names (endonyms)
static const char * const LANG_NAMES[LANG_COUNT] = {
    " English",
    " Italiano",
    " 中文", // “Chinese” in Chinese
    " Français",
    " Русский",   // Russian
    " Español",
    " Deutsch",
    " 日本語",       // Japanese
    " 한국어",        // Korean
    " Português",
    " العربية", // Arabic
    " Nederlands",      // Dutch
    " Slovenščina"    // Slovenian
};

// “Please make up room”
static const char * const MSG_MAKE_UP_ROOM[LANG_COUNT] = {
    "Please clean room",         // EN
    "Per favore, pulizia",       // IT
    "请打扫房间",                 // ZH (also ok: u8"请清理房间")
    "S'il vous plaît, ménage",   // FR or "SVP, ménage"
    "Пожалуйста, уборка",        // RU
    "Por favor, limpieza",       // ES
    "Bitte Reinigung",           // DE
    "清掃お願いします",           // JA (very short “Please cleaning”)
    "청소 부탁합니다",            // KO (“Please clean” polite)
    "Por favor, limpeza",        // PT
    "يرجى تنظيف الغرفة",       // AR  (shorter alt: "يرجى التنظيف")
    "A.u.b. reinigen",          // NL (short & clear)
    "Prosimo čiščenje"          // SL (short for “Please, cleaning”)
};

// “Do not disturb”
static const char * const MSG_DO_NOT_DISTURB[LANG_COUNT] = {
    "Do Not Disturb",       // EN
    "Non disturbare",       // IT
    "请勿打扰",              // ZH
    "Ne pas déranger",      // FR
    "Не беспокоить",        // RU
    "No molestar",          // ES
    "Bitte nicht stören",   // DE
    "起こさないでください",   // JA (standard hotel phrasing)
    "방해하지 마세요",       // KO (standard DND)
    "Não incomodar",        // PT
    "عدم الإزعاج",          // AR
    "Niet storen",          // NL
    "Ne moti"               // SL
};

// *********************************************************************************
// embed the image as a C array from LVGL's image converter:
// LV_IMG_DECLARE(splash_480x640); // 480x640, RGB565, no alpha

// *********************************************************************************

static lv_style_t styleButtonMmr;
static lv_style_t styleButtonDnd;
static lv_style_t styleButtonSettings;
static lv_style_t styleScreen;
static lv_style_t styleLabelMmr;
static lv_style_t styleLabelDnd;
static lv_style_t styleLabelSettings;

static lv_style_t style_radio;
static lv_style_t style_radio_chk;
static uint32_t active_language_index = 0;

static lv_obj_t *button_clean_my_room;
lv_obj_t *button_do_not_disturb;
lv_obj_t *button_settings;
static lv_obj_t *Backlight_slider;
static lv_obj_t * label_dnd;
static lv_obj_t * label_mmr;
static lv_obj_t * label_settings;

static lv_obj_t * screen_home;
static lv_obj_t * screen_settings;

lv_obj_t *splash_img;

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

static void button_settings_event_cb(lv_event_t * e)
{
  lv_event_code_t code = lv_event_get_code(e);
  // lv_obj_t * btn = lv_event_get_target(e);
  if(code == LV_EVENT_CLICKED)
  {
    Set_EXIO(BUZZER_PIN, HIGH);
    delay(100);
    Set_EXIO(BUZZER_PIN, LOW);

    lv_scr_load(screen_settings);
  }
}

// *********************************************************************************

void initStyles()
{
  lv_style_init(&styleButtonDnd);
  lv_style_init(&styleButtonMmr);
  lv_style_init(&styleLabelDnd);
  lv_style_init(&styleLabelMmr);
  lv_style_init(&styleLabelSettings);

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

  lv_style_set_text_font(&styleLabelSettings, font_icon);
  lv_obj_add_style(label_settings, &styleLabelSettings, LV_STATE_DEFAULT);

  lv_style_set_bg_color(&styleButtonSettings, lv_color_hex(COLOR_BG_BUTTON_INACTIVE));
  lv_obj_add_style(button_settings, &styleButtonSettings, LV_STATE_DEFAULT);

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

void languageChanged(int index)
{
  lv_label_set_text(label_mmr, MSG_MAKE_UP_ROOM[index]);
  lv_label_set_text(label_dnd, MSG_DO_NOT_DISTURB[index]);

  lv_style_set_text_font(&styleLabelDnd, fonts[index]);
  lv_style_set_text_font(&styleLabelMmr, fonts[index]);

  lv_obj_report_style_change(&styleLabelMmr);
  lv_obj_report_style_change(&styleLabelDnd);
}

// *********************************************************************************

void buildUiPanel()
{
  lv_style_set_bg_color(&styleScreen, lv_color_hex(COLOR_BG_SCREEN));
  lv_obj_report_style_change(&styleScreen);

  button_clean_my_room = lv_btn_create(lv_scr_act());
  lv_obj_set_pos(button_clean_my_room, 20, 20);
  lv_obj_set_size(button_clean_my_room, 440, 240);
  lv_obj_add_event_cb(button_clean_my_room, button_clean_my_room_event_cb, LV_EVENT_ALL, NULL);

  label_mmr = lv_label_create(button_clean_my_room);
  // lv_label_set_text(label_mmr, "Please clean Room");
  lv_obj_center(label_mmr);

  button_do_not_disturb = lv_btn_create(lv_scr_act());
  lv_obj_set_pos(button_do_not_disturb, 20, 310);
  lv_obj_set_size(button_do_not_disturb, 440, 240);
  lv_obj_add_event_cb(button_do_not_disturb, button_do_not_disturb_event_cb, LV_EVENT_ALL, NULL);

  label_dnd = lv_label_create(button_do_not_disturb);
  // lv_label_set_text(label_dnd, "Do not Disturb");
  lv_obj_center(label_dnd);
  
  Backlight_slider = lv_slider_create(lv_scr_act());
  lv_obj_set_pos(Backlight_slider, 20, 580);
  lv_obj_add_flag(Backlight_slider, LV_OBJ_FLAG_CLICKABLE);    
  lv_obj_set_size(Backlight_slider, 320, 45);              
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
  
  button_settings = lv_btn_create(lv_scr_act());
  lv_obj_set_pos(button_settings, 400, 575);
  lv_obj_set_size(button_settings, 55, 55);
  lv_obj_add_event_cb(button_settings, button_settings_event_cb, LV_EVENT_ALL, NULL);

  label_settings = lv_label_create(button_settings);
  lv_label_set_text(label_settings, LV_SYMBOL_SETTINGS);
  lv_obj_center(label_settings);

  languageChanged(LANG_EN);
  initStyles();
}

// *********************************************************************************

void radio_event_handler(lv_event_t * e)
{
  uint32_t * active_id = (uint32_t*)lv_event_get_user_data(e);
  lv_obj_t * cont = lv_event_get_current_target(e);
  lv_obj_t * act_cb = lv_event_get_target(e);
  lv_obj_t * old_cb = lv_obj_get_child(cont, *active_id);

  /*Do nothing if the container was clicked*/
  if(act_cb == cont) return;

  Set_EXIO(BUZZER_PIN, HIGH);
  delay(100);
  Set_EXIO(BUZZER_PIN, LOW);
  
  lv_obj_clear_state(old_cb, LV_STATE_CHECKED);   /*Uncheck the previous radio button*/
  lv_obj_add_state(act_cb, LV_STATE_CHECKED);     /*Uncheck the current radio button*/

  *active_id = lv_obj_get_index(act_cb);

  languageChanged(active_language_index);

  lv_scr_load(screen_home);

  // Serial.printf("Selected radio buttons: %d, %d\n\r", (int)active_language_index);
}

// *********************************************************************************

static void radiobutton_create(lv_obj_t * parent, int i)
{
  lv_obj_t * button = lv_checkbox_create(parent);
  lv_checkbox_set_text(button, LANG_NAMES[i]);
  lv_obj_set_size(button, 300, 50);
  lv_obj_set_pos(button, 10, 10 + 100 * i);

  lv_obj_set_style_text_color(button, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_color(button, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_CHECKED);
                            
  lv_obj_add_flag(button, LV_OBJ_FLAG_EVENT_BUBBLE);

  // lv_style_set_text_font(&style_radio, font_large, LV_PART_MAIN);
  // lv_style_set_text_font(&style_radio_chk, font_large);

  if(0 && language_is_rtl[i])
  {
    // lv_obj_set_style_base_dir(button, LV_BASE_DIR_RTL, LV_PART_MAIN);     // RTL text flow
    // lv_obj_set_style_text_align(button, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN);
  }
  else
  {
    lv_obj_set_style_base_dir(button, LV_BASE_DIR_LTR, LV_PART_MAIN);     // RTL text flow
    lv_obj_set_style_text_align(button, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
  }

  lv_obj_set_style_text_font(button, fonts[i], LV_PART_MAIN);

  lv_obj_add_style(button, &style_radio, LV_PART_INDICATOR);
  lv_obj_add_style(button, &style_radio_chk, LV_PART_INDICATOR | LV_STATE_CHECKED);
}

// *********************************************************************************

void buildSettingsScreen()
{
  // Settings screen
  screen_settings = lv_obj_create(NULL);
  lv_obj_add_style(screen_settings, &styleScreen, LV_STATE_DEFAULT);

  lv_style_init(&style_radio);
  lv_style_set_radius(&style_radio, LV_RADIUS_CIRCLE);

  lv_style_init(&style_radio_chk);
  lv_style_set_bg_img_src(&style_radio_chk, NULL);

  uint32_t i;
  char buf[32];

  lv_obj_t * radio_container = lv_obj_create(screen_settings);
  // lv_obj_set_flex_flow(radio_container, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_style_bg_opa(radio_container, LV_OPA_0, LV_PART_MAIN);
  lv_obj_set_style_border_opa(radio_container, LV_OPA_0, LV_PART_MAIN);
  lv_obj_set_size(radio_container, 440, 600);
  lv_obj_center(radio_container);
  // lv_obj_set_style_bg_color(radio_container, LV_COLOR_SCREEN_TRANSP, LV_PART_ANY);

  lv_obj_add_event_cb(radio_container, radio_event_handler, LV_EVENT_CLICKED, &active_language_index);

  for(int i = 0; i < LANG_COUNT; i++)
  {
    radiobutton_create(radio_container, i);
  }

  // Make the first checkbox checked
  lv_obj_add_state(lv_obj_get_child(radio_container, 0), LV_STATE_CHECKED);
}

// *********************************************************************************
// Never called
// *********************************************************************************

void close_ui_panel()
{
  lv_obj_clean(lv_scr_act());

  // lv_style_reset(&style_text_muted);
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
  // lv_obj_t *splash = (lv_obj_t *)t->user_data;
  lv_timer_del(t); // one-shot
  lv_obj_del(splash_img); // free splash screen

  buildUiPanel(); // go to normal UI
  buildSettingsScreen();
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
  lv_style_init(&styleScreen);

  lv_style_set_bg_color(&styleScreen, lv_color_hex(0xed1d24));
  lv_obj_add_style(lv_scr_act(), &styleScreen, LV_STATE_DEFAULT);

  static lv_style_t style;
  lv_style_init(&style);

  screen_home = lv_scr_act();

  splash_img = lv_img_create(lv_scr_act());
  lv_obj_add_style(splash_img, &style, 0);

  LV_IMG_DECLARE(splash_480x640); // 480x640, RGB565, no alpha
  lv_img_set_src(splash_img, &splash_480x640);
  lv_obj_center(splash_img);

  // keep ~2s, then switch to your main UI
  lv_timer_create(splash_done_cb, 2000, splash_img);
}

// *********************************************************************************
// *********************************************************************************
