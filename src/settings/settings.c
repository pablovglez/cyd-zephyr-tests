//
// Created by efisio on 11/12/25.
//
#include <zephyr/fs/fs.h>
#include <zephyr/fs/littlefs.h>
#include <zephyr/storage/flash_map.h>
#include "settings_names.h"
#include "settings.h"

LOG_MODULE_REGISTER(settings);
static int littlefs_flash_erase(unsigned int id)
{
    const struct flash_area *pfa;
    int rc;

    rc = flash_area_open(id, &pfa);
    if (rc < 0) {
        LOG_ERR("FAIL: unable to find flash area %u: %d\n",
            id, rc);
        return rc;
    }

    LOG_PRINTK("Area %u at 0x%x on %s for %u bytes\n",
           id, (unsigned int)pfa->fa_off, pfa->fa_dev->name,
           (unsigned int)pfa->fa_size);
    if (IS_ENABLED(CONFIG_APP_WIPE_STORAGE)) {
        rc = flash_area_flatten(pfa, 0, pfa->fa_size);
        LOG_ERR("Erasing flash area ... %d", rc);
    }

    flash_area_close(pfa);
    return rc;
}

static int littlefs_mount(struct fs_mount_t *mp)
{
    int rc;
    rc = littlefs_flash_erase((uintptr_t)mp->storage_dev);
    if (rc < 0) {
        return rc;
    }
    rc = fs_mount(mp);
    if (rc < 0) {
        LOG_PRINTK("FAIL: mount id %" PRIuPTR " at %s: %d\n",
               (uintptr_t)mp->storage_dev, mp->mnt_point, rc);
        return rc;
    }
    LOG_PRINTK("%s mount: %d\n", mp->mnt_point, rc);

    return 0;
}

int load_persistent_settings(PersistentSettings *params_p) {
    ParamEnum next = PROJECT_NAME;
    char line[CONF_LINE_SIZE];
    char fname[255];
    uint8_t buf[256];
    ssize_t nread;
    size_t line_pos = 0;
    int rc;

    FS_LITTLEFS_DECLARE_DEFAULT_CONFIG(storage);
    static struct fs_mount_t lfs_storage_mnt = {
        .type = FS_LITTLEFS,
        .fs_data = &storage,
        .storage_dev = (void *)FIXED_PARTITION_ID(storage_partition),
        .mnt_point = "/lfs",
    };
    struct fs_mount_t *mountpoint = &lfs_storage_mnt;
    rc = littlefs_mount(mountpoint);
    if (rc < 0) {
        return 0;
    }

    snprintf(fname, sizeof(fname), "%s/settings/settings.conf", mountpoint->mnt_point);

    struct fs_file_t file;
    fs_file_t_init(&file);
    rc = fs_open(&file, fname, FS_O_READ);
    if (rc < 0) {
        LOG_ERR("FAIL: open %s: %d", fname, rc);
        fs_unmount(mountpoint);
        return 0;
    }

    rc = fs_open(&file, fname, FS_O_READ);

    while ((nread = fs_read(&file, buf, sizeof(buf))) > 0) {
        for (ssize_t i = 0; i < nread; i++) {
            uint8_t c = buf[i];

            if (c == '\n') {
                /* terminate and process line */
                line[line_pos] = '\0';
                /* strip trailing CR if present */
                if (line_pos > 0 && line[line_pos - 1] == '\r') {
                    line[line_pos - 1] = '\0';
                }
                line_pos = 0;

                /* skip empty and comment lines */
                if (line[0] == '\0' || line[0] == ';' || line[0] == '#') {
                    continue;
                }

                const char *val = strrchr(line, '=');
                if (val && *(val + 1) != '\0') {
                    val++; /* point after '=' */
                    switch (next) {
                        case PROJECT_NAME:
                            strncpy(params_p->project_name, val, sizeof(params_p->project_name) - 1);
                            params_p->project_name[sizeof(params_p->project_name) - 1] = '\0';
                            next = WF_SSID;
                            break;
                        case WF_SSID:
                            strncpy(params_p->wifi_ssid, val, sizeof(params_p->wifi_ssid) - 1);
                            params_p->wifi_ssid[sizeof(params_p->wifi_ssid) - 1] = '\0';
                            next = WF_PASS;
                            break;
                        case WF_PASS:
                            strncpy(params_p->wifi_pass, val, sizeof(params_p->wifi_pass) - 1);
                            params_p->wifi_pass[sizeof(params_p->wifi_pass) - 1] = '\0';
                            next = WF_MAX_RETRY;
                            break;
                        case WF_MAX_RETRY:
                            params_p->wifi_max_retry = atoi(val);
                            next = WF_SHORT_RETRY_DELAY;
                            break;
                        case WF_SHORT_RETRY_DELAY:
                            params_p->wifi_short_retry_delay = atoi(val);
                            next = WF_LONG_RETRY_DELAY;
                            break;
                        case WF_LONG_RETRY_DELAY:
                            params_p->wifi_long_retry_delay = atoi(val);
                            next = PARAM_END;
                            break;
                        case PARAM_END:
                        default:
                            break;
                    }
                }
            } else {
                /* append to current line if space permits */
                if (line_pos < (CONF_LINE_SIZE - 1)) {
                    line[line_pos++] = (char)c;
                } else {
                    /* line overflow: truncate (or handle error) */
                    /* keep discarding until newline; still null-terminate */
                    line[CONF_LINE_SIZE - 1] = '\0';
                }
            }
        }
    }

    // Print the project name to confirm it was read correctly
    LOG_INF("Loaded Settings - Project Name: %s\n", params_p->project_name);

    fs_close(&file);
    rc = fs_unmount(mountpoint);

    return rc;
}