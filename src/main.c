//#include "display/display_touch.h"
#include "display/display.h"
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/logging/log_ctrl.h>
#include <zephyr/fs/fs.h>
#include <zephyr/fs/littlefs.h>
#include <zephyr/storage/flash_map.h>
#include <lvgl.h>

LOG_MODULE_REGISTER(main);
void print_partition_info(void)
{
    const struct flash_area *fa;
    int rc;

    /* Get slot0 partition info */
    rc = flash_area_open(FIXED_PARTITION_ID(slot0_partition), &fa);
    if (rc == 0) {
        printk("Slot0: offset=0x%x, size=0x%x\n", fa->fa_off, fa->fa_size);
        flash_area_close(fa);
    }

    /* Get slot1 partition info */
    rc = flash_area_open(FIXED_PARTITION_ID(slot1_partition), &fa);
    if (rc == 0) {
        printk("Slot1: offset=0x%x, size=0x%x\n", fa->fa_off, fa->fa_size);
        flash_area_close(fa);
    }

    /* Get storage partition info */
    rc = flash_area_open(FIXED_PARTITION_ID(storage_partition), &fa);
    if (rc == 0) {
        printk("Storage: offset=0x%x, size=0x%x\n", fa->fa_off, fa->fa_size);
        flash_area_close(fa);
    }
}


int main(void) {

    // Enable touch display function from display_touch.c
    /*
    int ret = 0;
    ret=draw_content_touch();

    return ret;
*/
    (void)log_set_tag("cyd-zephyr-tests"); // Add once
    LOG_INF("App cyd-zephyr-tests started.");
    print_partition_info();


    lv_init();

    init_display();

    draw_content();

    while (true) {
        update_display();
    }

}