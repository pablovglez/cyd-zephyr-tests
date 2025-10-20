#include "display.h"
#include <font/lv_font.h>
#include <lvgl.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/display.h>
#include <zephyr/drivers/gpio.h>
#include <stdio.h>
#include <stdbool.h>

static const struct gpio_dt_spec backlight = GPIO_DT_SPEC_GET(DT_ALIAS(backlight), gpios);

// Settings
static const uint32_t sleep_time_ms = 50; // Target 20 FPS

// content
uint32_t count = 0;
char buf[11] = {0};

lv_obj_t *hello_label;
lv_obj_t *counter_label;
lv_obj_t *rect;
lv_obj_t *circle;
lv_obj_t *circle2;
//lv_obj_t *button;
lv_style_t rect_style;
lv_style_t circle_style;
lv_point_t rect_points[5] = {{0, 0}, {120, 0}, {120, 20}, {0, 20}, {0, 0}};
const uint32_t circle_radius = 15;

void init_display() {
    const struct device *display;

    // Initialize the display
    display = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
    if (!device_is_ready(display)) {
        printk("Error: display not ready\r\n");
    }

    // Disable display blanking
    display_blanking_off(display);

    // activate Backlight
    if (gpio_is_ready_dt(&backlight)) {
        gpio_pin_configure_dt(&backlight, GPIO_OUTPUT);
        gpio_pin_set_dt(&backlight, true);
    }
}

static void btn_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * btn = lv_event_get_target_obj(e);
    if(code == LV_EVENT_CLICKED) {
        static uint8_t cnt = 0;
        cnt++;

        /*Get the first child of the button which is the label and change its text*/
        lv_obj_t * label = lv_obj_get_child(btn, 0);
        lv_label_set_text_fmt(label, "Button: %d", cnt);
    }
}

void draw_content() {
    static bool created = false;
    if (created) {
        return;
    }
    created = true;

    /* Remove any existing children from the active screen so we don't stack
     * multiple element sets when this is accidentally called more than once. */
    lv_obj_clean(lv_scr_act());

    /* Make sure the screen background is fully painted so old framebuffer
     * content doesn't show through (some drivers expose remnants otherwise). */
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0xCCF5FF), 0);
    /*
    Sets the background opacity style on the active screen object.
    lv_scr_act() returns the current active screen (lv_obj_t *).
    LV_OPA_COVER is full opacity (255). (LV_OPA_TRANSP = 0 means fully transparent.)
    The third argument 0 is the selector — it targets the main part and default state (common shorthand).
     */
    lv_obj_set_style_bg_opa(lv_scr_act(), LV_OPA_COVER, 0);

    /* Query display resolution so we can place objects precisely. This is
     * more robust than relying solely on lv_obj_align when screen sizes or
     * drivers behave differently. */

    // Create a rectangle as a bordered object (120x20) first so labels are on top
    rect = lv_obj_create(lv_scr_act());
    lv_obj_set_size(rect, 160, 20);
    lv_obj_set_style_bg_opa(rect, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_color(rect, lv_color_hex(0xFFA100), 0);
    lv_obj_set_style_border_width(rect, 3, 0);
    /* Align rectangle to the top middle (more robust than manual x calculation) */
    lv_obj_align(rect, LV_ALIGN_TOP_MID, 0, 0);

    // Create an object with a circular appearance centered on screen
    const lv_coord_t dia = (lv_coord_t) (circle_radius * 2);
    circle = lv_obj_create(lv_scr_act());
    lv_obj_set_size(circle, dia, dia);
    lv_obj_set_style_bg_color(circle, lv_color_hex(0xFF0000), 0);
    lv_obj_set_style_bg_opa(circle, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(circle, circle_radius, 0);

    // Create an object with a circular appearance centered on screen
    circle2 = lv_obj_create(lv_scr_act());
    lv_obj_set_size(circle2, dia, dia);
    lv_obj_set_style_bg_color(circle2, lv_color_hex(0x00FF11), 0);
    lv_obj_set_style_bg_opa(circle2, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(circle2, circle_radius, 0);
    //mono_font.get_glyph_dsc = fix_w_get_glyph_dsc;

    hello_label = lv_label_create(lv_scr_act());
    lv_label_set_text(hello_label, "Que paso, Ponciano?");
    /* Ensure text is visible across themes */
    lv_obj_set_style_text_color(hello_label, lv_color_hex(0x000000), 0);
    lv_obj_align(hello_label, LV_ALIGN_TOP_MID, 0, 5);
    lv_obj_move_foreground(hello_label);

    // Create a dynamic label widget (bottom center)
    counter_label = lv_label_create(lv_scr_act());
    lv_label_set_text(counter_label, "0");
    lv_obj_set_style_text_color(counter_label, lv_color_hex(0x000000), 0);
    lv_obj_align(counter_label, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_move_foreground(counter_label);

    /**
     * Create a button with a label and react on click event.
    */
    /*
    button = lv_button_create(lv_screen_active()); //Add a button the current screen
    lv_obj_align(button, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_size(button, 120, 50); //Set its size

    // ensure the button background is visible
    lv_obj_set_style_bg_color(button, lv_color_hex(0x0800FF), 0);
    lv_obj_set_style_bg_opa(button, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(button, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_border_width(button, 2, 0);
    lv_obj_add_event_cb(button, btn_event_cb, LV_EVENT_ALL, NULL); //Assign a callback to the button

    lv_obj_t *label = lv_label_create(button); //Add a label to the button
    lv_label_set_text(label, "Button"); //Set the labels text
    lv_obj_center(label);

    if (lv_indev_get_next(NULL) == NULL) {
        printk("Warning: no LVGL input device registered; clicks will not be delivered\r\n");
    }

    */
}

void update_display() {
    /* Calculate how many frames correspond to ~1 second. Guard against a
     * zero division if sleep_time_ms is somehow zero. */
    static lv_coord_t xpos = 0, ypos = 5;
    static lv_coord_t xpos2= 205, ypos2 = 0;

    const uint32_t frames_per_second = (sleep_time_ms > 0) ? (1000u / sleep_time_ms) : 1u;
    const lv_coord_t hor = lv_disp_get_hor_res(NULL);
    const lv_coord_t ver = lv_disp_get_ver_res(NULL);
    //const lv_coord_t dia = (lv_coord_t) (circle_radius * 2);
    lv_coord_t max_x = (hor > circle_radius * 2) ? (hor - circle_radius * 2) : 0; //210
    lv_coord_t max_y = (ver > circle_radius * 2) ? (ver - circle_radius * 2) : 0;  //290
    //lv_coord_t max_x = 210;//(hor > circle_radius * 2) ? (hor - circle_radius * 2) : 0; //210
    //lv_coord_t max_y = 290; //(ver > circle_radius * 2) ? (ver - circle_radius * 2) : 0;  //290
    //static lv_coord_t xpos2 = 205, ypos2 = 0;
    lv_obj_align(circle, LV_ALIGN_TOP_LEFT, xpos, ypos);
    lv_obj_align(circle2, LV_ALIGN_TOP_LEFT, xpos2, ypos2);
    static lv_coord_t dx = 1, dy = 1;
    static lv_coord_t dx2 = -1, dy2 = 1;

    // Update counter label every second
    count++;
    if (frames_per_second > 0 && (count % frames_per_second) == 0) {
        /* Use snprintf to avoid buffer overruns and format as unsigned */
        snprintf(buf, sizeof(buf), "%u", count / frames_per_second);
        lv_label_set_text(counter_label, buf);
    }

    /* Move the circle diagonally
     * Coordinates are to be considered with the USB port at the bottom*/

    xpos += dx;
    xpos2 += dx2;
    ypos += dy;
    ypos2 += dy2;

    /* Ensure direction */
    if (xpos == 0) {
        dx = (dx < 0) ? -dx : dx; // ensure positive
    } else if (xpos == max_x) {
        dx = (dx > 0) ? -dx : dx; // ensure negative
    }

    if (ypos == 0) {
        dy = (dy < 0) ? -dy : dy; // ensure positive
    } else if (ypos == max_y) {
        dy = (dy > 0) ? -dy : dy; // ensure negative
    }

    /* Ensure direction */
    if (xpos2 == 0) {
        dx2 = (dx2 < 0) ? -dx2 : dx2; // ensure positive
    } else if (xpos2 == max_x) {
        dx2 = (dx2 > 0) ? -dx2 : dx2; // ensure negative
    }

    if (ypos2 == 0) {
        dy2 = (dy2 < 0) ? -dy2 : dy2; // ensure positive
    } else if (ypos2 == max_y) {
        dy2 = (dy2 > 0) ? -dy2 : dy2; // ensure negative
    }

    lv_obj_set_pos(circle, xpos, ypos);
    lv_obj_set_pos(circle2, xpos2, ypos2);
    // Must be called periodically
    lv_task_handler();
    printk("Circle 2 positions (%d, %d)\n", xpos2, ypos2);

    k_msleep(sleep_time_ms);
}
