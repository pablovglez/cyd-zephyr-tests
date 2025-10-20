#if defined CONFIG_CYD_ENABLE_TOUCH
    #include "display/display_touch.h"
#else
    #include "display/display.h"
#endif
#include <zephyr/kernel.h>
#include <lvgl.h>


int main(void) {

#if defined CONFIG_CYD_ENABLE_TOUCH
    int ret = 0;
    ret=draw_content_touch();

    return ret;
#else
    lv_init();

    init_display();

    draw_content();

    while (true) {
        update_display();
    }
#endif

}