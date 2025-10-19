#include "display/display.h"
#include <zephyr/kernel.h>
#include <lvgl.h>


int main(void)
{
    lv_init();

    init_display();

    draw_content();

    while (true) {
        update_display();
    }
}