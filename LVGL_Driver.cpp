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

// Note: The provided LVGL library file must be installed first
// Do not use library from library manager
// do not use standard lvgl_conf.h

#include "LVGL_Driver.h"

lv_disp_drv_t disp_drv;

static lv_disp_draw_buf_t draw_buf;
void* buf1 = NULL;
void* buf2 = NULL;
// static lv_color_t buf1[ LVGL_BUF_LEN ];
// static lv_color_t buf2[ LVGL_BUF_LEN ];
// static lv_color_t* buf1 = (lv_color_t*) heap_caps_malloc(LVGL_BUF_LEN, MALLOC_CAP_SPIRAM);
// static lv_color_t* buf2 = (lv_color_t*) heap_caps_malloc(LVGL_BUF_LEN, MALLOC_CAP_SPIRAM);

/* Serial debugging */
void Lvgl_print(const char * buf)
{
    // Serial.printf(buf);
    // Serial.flush();
}

/*  Display flushing 
    Displays LVGL content on the LCD
    This function implements associating LVGL data to the LCD screen
*/
void Lvgl_Display_LCD( lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p )
{
  LCD_addWindow(area->x1, area->y1, area->x2, area->y2, ( uint8_t *)&color_p->full);
  lv_disp_flush_ready( disp_drv );
}

/*Read the touchpad*/
void Lvgl_Touchpad_Read( lv_indev_drv_t * indev_drv, lv_indev_data_t * data )
{
  uint16_t touchpad_x[GT911_LCD_TOUCH_MAX_POINTS] = {0};
  uint16_t touchpad_y[GT911_LCD_TOUCH_MAX_POINTS] = {0};
  uint16_t strength[GT911_LCD_TOUCH_MAX_POINTS]   = {0};
  uint8_t touchpad_cnt = 0;
  Touch_Read_Data();
  uint8_t touchpad_pressed = Touch_Get_XY(touchpad_x, touchpad_y, strength, &touchpad_cnt, GT911_LCD_TOUCH_MAX_POINTS);
  if (touchpad_pressed && touchpad_cnt > 0) {
    data->point.x = touchpad_x[0];
    data->point.y = touchpad_y[0];
    data->state = LV_INDEV_STATE_PR;
    printf("LVGL : X=%u Y=%u num=%d\r\n", touchpad_x[0], touchpad_y[0],touchpad_cnt);
  } else {
    data->state = LV_INDEV_STATE_REL;
  }
}

void lvgl_tick_update(void *arg)
{
    /* Tell LVGL how many milliseconds has elapsed */
    lv_tick_inc(EXAMPLE_LVGL_TICK_PERIOD_MS);
}

void Lvgl_Init(void)
{
  lv_init();
  // esp_lcd_rgb_panel_get_frame_buffer(panel_handle, 2, &buf1, &buf2);                                          
  
  buf1 = (lv_color_t*) heap_caps_malloc(LVGL_BUF_LEN, MALLOC_CAP_SPIRAM);
  buf2 = (lv_color_t*) heap_caps_malloc(LVGL_BUF_LEN, MALLOC_CAP_SPIRAM);
  lv_disp_draw_buf_init( &draw_buf, buf1, buf2, LVGL_WIDTH * LVGL_HEIGHT);                    

  /*Initialize the display*/
  lv_disp_drv_init( &disp_drv );
  /*Change the following line to your display resolution*/
  disp_drv.hor_res = LVGL_WIDTH;
  disp_drv.ver_res = LVGL_HEIGHT;
  disp_drv.flush_cb = Lvgl_Display_LCD;
  // disp_drv.full_refresh = 1;                                                                                  
  disp_drv.draw_buf = &draw_buf;
  disp_drv.user_data = panel_handle;
  lv_disp_drv_register( &disp_drv );

  /*Initialize the (dummy) input device driver*/
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init( &indev_drv );
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = Lvgl_Touchpad_Read;
  lv_indev_drv_register( &indev_drv );

  /* Create simple label */
  lv_obj_t *label = lv_label_create( lv_scr_act() );
  lv_label_set_text( label, "Hello Arduino and LVGL!");
  lv_obj_align( label, LV_ALIGN_CENTER, 0, 0 );

  const esp_timer_create_args_t lvgl_tick_timer_args = {
    .callback = &lvgl_tick_update,
    .name = "lvgl_tick"
  };
  esp_timer_handle_t lvgl_tick_timer = NULL;
  esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer);
  esp_timer_start_periodic(lvgl_tick_timer, EXAMPLE_LVGL_TICK_PERIOD_MS * 1000);

}

void Lvgl_Loop(void)
{
  lv_timer_handler(); /* let the GUI do its work */
}
