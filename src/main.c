//#include "display/display_touch.h"
#include "display/display.h"
#include <zephyr/kernel.h>
#include <lvgl.h>


int main(void) {

    // Enable touch display function from display_touch.c
    /*
    int ret = 0;
    ret=draw_content_touch();

    return ret;
*/

    lv_init();

    init_display();

    draw_content();

    while (true) {
        update_display();
    }

}