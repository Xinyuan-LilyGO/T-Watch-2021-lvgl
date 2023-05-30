#include "Arduino.h"
#include "TFT_eSPI.h" /* Please use the TFT library provided in the library. */
#include "CST816S.h"
#include "config.h"
#include "lvgl.h"

LV_IMG_DECLARE(logo);
#define TWATCH_TOUCH_RES 33
#define TWATCH_TOUCH_INT 35
#define TWATCH_IICSCL 25
#define TWATCH_IICSDA 26

static const uint16_t screenWidth = 240;
static const uint16_t screenHeight = 240;
TFT_eSPI tft = TFT_eSPI(screenWidth, screenHeight);

#define WAIT 1000

void lvgl_test(void);
void lv_example_calendar_1(lv_obj_t *parent);

unsigned long targetTime = 0; 
extern uint8_t logo_map[];
static lv_color_t *buf = NULL;
static lv_disp_draw_buf_t draw_buf;
static lv_obj_t *dis = NULL;
static lv_obj_t *clocker_obj = NULL;
static lv_meter_indicator_t *indic_seconds;
static lv_meter_indicator_t *indic_min;
static lv_meter_indicator_t *indic_hour;
static lv_obj_t *meter;
uint32_t hour_t;
uint32_t min_t;
uint32_t sec_t;
lv_timer_t *timer;
CST816S_Class touch;

void my_print(const char *buf) {
  Serial.printf(buf);
  Serial.flush();
}

void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area,
                   lv_color_t *color_p) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);
  tft.pushImage(area->x1, area->y1, w, h, (uint16_t *)&color_p->full);
  lv_disp_flush_ready(disp);
}

/*Read the touchpad*/
void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data) {
    if (touch.read()) {
      uint16_t x = touch.getX();
      uint16_t y = touch.getY();

      data->state = LV_INDEV_STATE_PR;
      data->point.x = x;
      data->point.y = y;
  }
  else 
  {
      data->state = LV_INDEV_STATE_REL;
  }
}

void scan(void) {
  uint8_t err, addr;
  int nDevices = 0;
  for (addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    err = Wire.endTransmission();
    if (err == 0) {
      Serial.print("I2C device found at address 0x");
      if (addr < 16)
        Serial.print("0");
      Serial.print(addr, HEX);
      Serial.println(" !");
      nDevices++;
    } else if (err == 4) {
      Serial.print("Unknow error at address 0x");
      if (addr < 16)
        Serial.print("0");
      Serial.println(addr, HEX);
    }
  }
  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("done\n");
}

void setup() {
  Serial.begin(115200);
  Serial.println("Hello T-HMI");
  pinMode(4, ANALOG);
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);

  Wire.begin(TWATCH_IICSDA, TWATCH_IICSCL);
  scan();

  touch.begin(Wire, TWATCH_TOUCH_RES, TWATCH_TOUCH_INT);

  tft.begin();
  tft.setRotation(0);
  tft.setSwapBytes(true);
  tft.pushImage(0, 0, 240, 240, (uint16_t *)logo_map);

  delay(1000);
  lv_init();

  size_t buffer_size =
      sizeof(lv_color_t) * EXAMPLE_LCD_H_RES * EXAMPLE_LCD_V_RES;
  buf = (lv_color_t *)ps_malloc(buffer_size);
  if (buf == NULL) {
    while (1) {
      Serial.println("buf NULL");
      delay(500);
    }
  }
  lv_disp_draw_buf_init(&draw_buf, buf, NULL, buffer_size);

  /*Initialize the display*/
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  /*Change the following line to your display resolution*/
  disp_drv.hor_res = EXAMPLE_LCD_H_RES;
  disp_drv.ver_res = EXAMPLE_LCD_V_RES;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  /*Initialize the (dummy) input device driver*/
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touchpad_read;
  lv_indev_drv_register(&indev_drv);

  lvgl_test();

  Serial.println("lvgl_test end");
}

void lvgl_test(void) {
  dis = lv_tileview_create(lv_scr_act());
  lv_obj_align(dis, LV_ALIGN_TOP_RIGHT, 0, 0);
  lv_obj_set_size(dis, LV_PCT(100), LV_PCT(100));
  lv_obj_set_style_bg_color(dis, lv_color_hex(0x000000), LV_PART_MAIN);

  lv_obj_t *tv1 = lv_tileview_add_tile(dis, 0, 0, LV_DIR_VER);
  lv_obj_t *tv2 = lv_tileview_add_tile(dis, 0, 1, LV_DIR_VER);

  clock_display(tv1);
  lv_example_calendar_1(tv2);
}

static void event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_current_target(e);

    if(code == LV_EVENT_VALUE_CHANGED) {
        lv_calendar_date_t date;
        if(lv_calendar_get_pressed_date(obj, &date)) {
            LV_LOG_USER("Clicked date: %02d.%02d.%d", date.day, date.month, date.year);
        }
    }
}

void lv_example_calendar_1(lv_obj_t *parent)
{
    lv_obj_t  * calendar = lv_calendar_create(parent);
    lv_obj_set_size(calendar, 185, 185);
    lv_obj_align(calendar, LV_ALIGN_CENTER, 0, 27);
    lv_obj_add_event_cb(calendar, event_handler, LV_EVENT_ALL, NULL);

    lv_calendar_set_today_date(calendar, 2021, 02, 23);
    lv_calendar_set_showed_date(calendar, 2021, 02);

    /*Highlight a few days*/
    static lv_calendar_date_t highlighted_days[3];       /*Only its pointer will be saved so should be static*/
    highlighted_days[0].year = 2021;
    highlighted_days[0].month = 02;
    highlighted_days[0].day = 6;

    highlighted_days[1].year = 2021;
    highlighted_days[1].month = 02;
    highlighted_days[1].day = 11;

    highlighted_days[2].year = 2022;
    highlighted_days[2].month = 02;
    highlighted_days[2].day = 22;

    lv_calendar_set_highlighted_dates(calendar, highlighted_days, 3);

#if LV_USE_CALENDAR_HEADER_DROPDOWN
    lv_calendar_header_dropdown_create(calendar);
#elif LV_USE_CALENDAR_HEADER_ARROW
    lv_calendar_header_arrow_create(calendar);
#endif
    lv_calendar_set_showed_date(calendar, 2021, 10);
}

void ifan_standby_time_event_cb(lv_timer_t *timer) {
  sec_t++;
  if (sec_t == 60) {
    min_t += 1;
    sec_t = 0;
  }
  if (min_t == 60) {
    hour_t += 1;
    min_t = 0;
  }
  if (hour_t == 13) {
    hour_t = 1;
  }
  if (meter && indic_seconds && indic_min && indic_hour) {
    lv_meter_set_indicator_value(meter, indic_seconds, sec_t);
    lv_meter_set_indicator_value(meter, indic_min, min_t);
    lv_meter_set_indicator_value(meter, indic_hour, hour_t);
  }
}

void clock_display(lv_obj_t *parent) {
  if (clocker_obj == NULL) {
    clocker_obj = lv_meter_create(parent);
    lv_obj_set_size(clocker_obj, 185, 185);
    lv_obj_set_style_border_width(clocker_obj, 0, 0);
    lv_obj_set_style_bg_color(clocker_obj, lv_color_black(), 0);
    lv_obj_set_style_pad_all(clocker_obj, 0, 0);
    lv_obj_set_pos(clocker_obj, 28, 28);
    lv_obj_set_scrollbar_mode(clocker_obj, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_scroll_dir(clocker_obj, LV_DIR_NONE);
    lv_obj_set_style_shadow_color(clocker_obj, lv_color_hex(0x2791c9), 0);
    lv_obj_set_style_shadow_opa(clocker_obj, LV_OPA_80, 0);
    lv_obj_set_style_shadow_spread(clocker_obj, 5, 0);
    lv_obj_set_style_shadow_width(clocker_obj, 5, 0);

    meter = lv_meter_create(clocker_obj);
    lv_obj_set_size(meter, 185, 185);
    lv_obj_set_style_pad_all(meter, 5, 0);
    lv_obj_set_style_bg_color(meter, lv_color_black(), 0);
    lv_obj_set_style_border_width(meter, 0, 0);
    lv_obj_center(meter);

    LV_IMG_DECLARE(centre_img)
    lv_obj_t *white_obj = lv_obj_create(meter);
    lv_obj_set_size(white_obj, 40, 40);
    lv_obj_set_style_bg_img_src(white_obj, &centre_img, 0);
    lv_obj_set_style_bg_opa(white_obj, 0, 0);
    lv_obj_set_style_border_opa(white_obj, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_pos(white_obj, 72 - 5, 72 - 4);

    /*Create a scale for the seconds*/
    lv_meter_scale_t *scale_seconds = lv_meter_add_scale(meter);
    lv_meter_set_scale_ticks(meter, scale_seconds, 61, 10, 75,
                             lv_color_black());
    lv_meter_set_scale_range(meter, scale_seconds, 0, 60, 360, 270);

    /*Create a scale for the minutes*/
    lv_meter_scale_t *scale_min = lv_meter_add_scale(meter);
    lv_meter_set_scale_ticks(meter, scale_min, 13, 3, 13,
                             lv_color_hex(0x29afee));
    lv_meter_set_scale_range(meter, scale_min, 0, 60, 360, 270);

    /*Create an other scale for the hours. It's only visual and contains only
     * major ticks*/
    lv_meter_scale_t *scale_hour = lv_meter_add_scale(meter);
    lv_meter_set_scale_ticks(meter, scale_hour, 5, 3, 13,
                             lv_color_hex(0x29afee));
    lv_meter_set_scale_range(meter, scale_hour, 0, 12, 360, 270);

    LV_IMG_DECLARE(hour_img)
    LV_IMG_DECLARE(sec_img)
    LV_IMG_DECLARE(min_img)
    /*Add a the hands from images*/
    indic_min = lv_meter_add_needle_img(meter, scale_min, &min_img, 20, 2);
    indic_hour = lv_meter_add_needle_img(meter, scale_hour, &hour_img, 1, 2);
    indic_seconds =
        lv_meter_add_needle_img(meter, scale_seconds, &sec_img, 20, 2);

    ifan_standby_time_event_cb(NULL);

    timer = lv_timer_create(ifan_standby_time_event_cb, 1000, NULL);
    lv_timer_ready(timer);
    lv_timer_reset(timer);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  lv_timer_handler();
  delay(2);

  static uint32_t last_tick;
  if (millis() - last_tick > 100) {
    last_tick = millis();
  }
}

