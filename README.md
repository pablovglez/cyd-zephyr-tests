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

## How to upload files to the board

1. Prepare the files to upload in a folder, e.g., `lfs/`
2. Use mklittlefs to create a littlefs image:
```bash
{{mklittlefs_path}}/mklittlefs/mklittlefs-4.1.0-5-g68af3b9-generic-linux64/mklittlefs -c lfs -s 0x30000 -p 0x100 littlefs.bin
```
Note: The size `-s` should match the size defined in the overlay file, e.g., DT_SIZE_K(192) -> 0x30000.
Note: The page size `-p` should match the page size defined in the overlay file, e.g., DT_PROP(DT_INST(0, zephyr_littlefs), page_size) -> 0x100.
3. Use `esptool.py` to upload the littlefs image to the board:
```bash
esptool.py --chip esp32 --port /dev/ttyUSB0 write_flash 0x3B0000 {{littlefs_bin_path}}/littlefs.bin
```
Note: The address `0x3B0000` should match the offset defined in the overlay file, e.g., reg = <0x3B0000 ... >;
