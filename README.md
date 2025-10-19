# cyd-zephyr-tests
Base project to use ESP Cheap Yellow Display with Zephyr

## How to build

```bash
 west build -p always -b esp32_devkitc_wroom/esp32/procpu -- -DDTC_OVERLAY_FILE=boards/esp32_devkitc_wroom.overlay
```

## How to flash
```bash
 west flash
```