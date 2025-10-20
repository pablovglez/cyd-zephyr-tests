//
// Created by pgonzalez on 10/20/25.
//

#include "display_touch.h"
#include <font/lv_font.h>
#include <lvgl.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/display.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/input/input.h>
#include <stdio.h>
#include <stdbool.h>

// Settings
static const int32_t sleep_time_ms = 50;        // Target 20 FPS
static const struct gpio_dt_spec backlight = GPIO_DT_SPEC_GET(DT_ALIAS(backlight), gpios);
int nStep = 10;
const int nMaxX = 140;
const int nMaxY = 100;
int nXPos = 0;
int nYPos = 0;

lv_point_t touch_point = {0, 0};

void lvgl_print_heap_info(bool dump_chunks);

int configure_led(const struct gpio_dt_spec *led)
{
	int ret;

	ret = gpio_is_ready_dt(led);
	if ( ! ret) {
		printk("gpio_is_ready_dt led not ready\r\n");
		return ret;
	}

	ret = gpio_pin_configure_dt(led, GPIO_OUTPUT);
	if (ret < 0) {
		printk("gpio_pin_configure_dt error: %d\r\n", ret);
		return ret;
	}

	ret = gpio_pin_set_dt(led, 0);
	if (ret < 0) {
		return ret;
	}
	return 0;
}

char* event_code_text(lv_event_code_t code)
{
    switch(code) {
        case LV_EVENT_PRESSED:
            return "LV_EVENT_PRESSED";
        case LV_EVENT_CLICKED:
            return "LV_EVENT_CLICKED";
        case LV_EVENT_LONG_PRESSED:
            return "LV_EVENT_LONG_PRESSED";
        case LV_EVENT_LONG_PRESSED_REPEAT:
            return "LV_EVENT_LONG_PRESSED_REPEAT";

        default:
            return "";
    }
}

char* position_text(int align)
{
    switch(align)
    {

        case LV_ALIGN_TOP_MID:    return "N";
        case LV_ALIGN_RIGHT_MID:  return "E";
        case LV_ALIGN_BOTTOM_MID: return "S";
        case LV_ALIGN_LEFT_MID:   return "W";
        case LV_ALIGN_CENTER:     return "C";

        default: return "O";
    }
}

void event_handler(struct _lv_event_t * event)
{
    lv_event_code_t code = lv_event_get_code(event);
    int ud = (int) lv_event_get_user_data(event);

    char * code_text = event_code_text(code);
    char * pos_text  = position_text(ud);

    if((code == LV_EVENT_CLICKED) || (code == LV_EVENT_LONG_PRESSED_REPEAT))
    {
        switch(*pos_text)
        {
            case 'N': nYPos -= nStep; break;
            case 'E': nXPos += nStep; break;
            case 'S': nYPos += nStep; break;
            case 'W': nXPos -= nStep; break;
            case 'C': nXPos = 0; nYPos = 0;break;
        }
        if (nXPos > nMaxX)  nXPos = -nMaxX;
        if (nXPos < -nMaxX) nXPos = nMaxX;
        if (nYPos < -nMaxY) nYPos = nMaxY;
        if (nYPos > nMaxY)  nYPos = -nMaxY;
    }
    // else if(code == LV_EVENT_VALUE_CHANGED) {
    //     //LV_LOG_USER("Toggled");
    // }

    if (*code_text)
    printk("Got Event %s with %s\r\n", code_text, pos_text);
}

lv_obj_t * create_button(int w, int h, int align, char* szLabel, void *user_data)
{
    lv_obj_t *btn = lv_btn_create(lv_scr_act());

    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_ALL, user_data);
    lv_obj_set_size(btn, w, h);
    lv_obj_align(btn, align, 0, 0);

    lv_obj_t * label = lv_label_create(btn);          /*Add a label to the button*/
    lv_label_set_text(label, szLabel);                     /*Set the labels text*/
    lv_obj_center(label);

    return btn;
}

static const struct device *const touch_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_touch));
static struct {
	size_t x;
	size_t y;
	bool pressed;
} touch_point2;



#define INPUT_BTN_TOUCH 0x14a           /**< Touchscreen touch */
#define INPUT_ABS_X 0x00                /**< Absolute X coordinate */
#define INPUT_ABS_Y 0x01                /**< Absolute Y coordinate */
#define INPUT_ABS_Z 0x02                /**< Absolute Z coordinate */

static void touch_event_callback(struct input_event *evt, void *user_data) {
    // swap x and y in lines below
    if (evt->code == INPUT_ABS_X) {
        touch_point2.y = evt->value;
    }
    if (evt->code == INPUT_ABS_Y) {
        touch_point2.x = evt->value;
    }
    if (evt->code == INPUT_BTN_TOUCH) {
        touch_point2.pressed = evt->value;
        printk(" touch at (%d, %d) (%d)\n\r", touch_point2.x, touch_point2.y, touch_point2.pressed);
    }
}

INPUT_CALLBACK_DEFINE(touch_dev, touch_event_callback, NULL);

int draw_content_touch() {
    int ret;

    printk("Starting app\r\n");
    uint32_t count = 0;
    char buf[100] = {0};
    const struct device *display;

    lv_obj_t *btnTopMid;
    lv_obj_t *btnRightMid;
    lv_obj_t *btnBottomMid;
    lv_obj_t *btnLeftMid;
    lv_obj_t *btnCenter;

    lv_obj_t *counter_label;
    lv_obj_t *circle;
    lv_style_t counter_label_style;
    lv_style_t circle_style;
    const uint32_t circle_radius = 15;

    int lastpressed = 0;

    if (0 != configure_led(&backlight)) {
        return 0;
    }

    //Set pin state
    ret = gpio_pin_set_dt(&backlight, 1);
    if (ret < 0) {
        return 0;
    }

    // Initialize the display
    display = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
    if (!device_is_ready(display)) {
        printk("Error: display not ready\r\n");
        return 0;
    }

    if (!device_is_ready(touch_dev)) {
        printk("Error: touch not ready\r\n");
        return 0;
    }

    // Create a static label widget
    btnTopMid = create_button(50, 50, LV_ALIGN_TOP_MID, "N", (void *) LV_ALIGN_TOP_MID);
    btnRightMid = create_button(50, 50, LV_ALIGN_RIGHT_MID, "E", (void *) LV_ALIGN_RIGHT_MID);
    btnBottomMid = create_button(50, 50, LV_ALIGN_BOTTOM_MID, "S", (void *) LV_ALIGN_BOTTOM_MID);
    btnLeftMid = create_button(50, 50, LV_ALIGN_LEFT_MID, "W", (void *) LV_ALIGN_LEFT_MID);
    btnCenter = create_button(50, 50, LV_ALIGN_CENTER, "C", (void *) LV_ALIGN_CENTER);

    // Adjust style for counter label
    lv_style_init(&counter_label_style);
    //lv_style_set_text_font(&counter_label_style, &lv_font_montserrat_20);

    // Create a dynamic label widget
    counter_label = lv_label_create(lv_scr_act());
    lv_obj_add_style(counter_label, &counter_label_style, 0);
    lv_obj_align(counter_label, LV_ALIGN_BOTTOM_MID, 0, -40);

    // Set circle style
    lv_style_init(&circle_style);
    lv_style_set_radius(&circle_style, circle_radius);
    lv_style_set_bg_opa(&circle_style, LV_OPA_100);
    lv_style_set_bg_color(&circle_style, lv_color_hex(0xFF0000));

    // Create an object with the new style
    circle = lv_obj_create(lv_scr_act());
    lv_obj_set_size(circle, circle_radius * 2, circle_radius * 2);
    lv_obj_add_style(circle, &circle_style, 0);
    lv_obj_align(circle, LV_ALIGN_CENTER, 0, 5);

    // Disable display blanking
    display_blanking_off(display);
#ifdef CONFIG_LV_Z_MEM_POOL_SYS_HEAP
    lvgl_print_heap_info(false);
#else
    printf("lvgl in malloc mode\n");
#endif
    // Do forever
    while (1) {
        // Update counter label every second
        count++;

        if (((count % (1000 / sleep_time_ms)) == 0) || (lastpressed != touch_point2.pressed)) {
            sprintf(buf, "Loop %d (%d, %d) %s\r\n",
                    count / (1000 / sleep_time_ms),
                    touch_point2.x, touch_point2.y,
                    (touch_point2.pressed ? "pressed" : "not pressed")
            );

            lv_label_set_text(counter_label, buf);
            lastpressed = touch_point2.pressed;
        }

        lv_obj_align(circle, LV_ALIGN_CENTER, (nXPos), (nYPos));

        // Must be called periodically
        lv_task_handler();

        // Sleep
        k_msleep(sleep_time_ms);
    }

    return 0;
}