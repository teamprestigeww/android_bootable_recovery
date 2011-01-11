#include <stdlib.h>
#include <stdio.h>

#include "recovery_lib.h"
#include "roots.h"
#include "recovery_ui.h"
#include "recovery_menu.h"

typedef struct {
    const char* name;
    const char* path;
} wipe_info;

#define WIPE_PARTITION_SYSTEM 0
#define WIPE_PARTITION_DATA   1
#define WIPE_PARTITION_BOOT   2
#define WIPE_PARTITION_CACHE  3
#define WIPE_PARTITION_MISC   4

void wipe_partition(int which, int confirm) {
    static wipe_info wipe_partitions[] = {
        { "System", "/system" },
        { "Data", "/data" },
        { "Boot", "/boot" },
        { "Cache", "/cache" },
        { "Misc", "/misc" }
    };
    static int num_partitions = sizeof(wipe_partitions) / sizeof(wipe_partitions[0]);

    // bail out if which is too big
    if (which >= num_partitions) {
        return;
    }

    if (confirm) {
        char** title_headers = NULL;

        char* confirm;
        char* confirm_select;

        // if we have a specific one, then use it
        const char* confirm_prefix = "Confirm wipe of ";
        const char* confirm_suffix = "?";
        const char* confirm_select_prefix = " Yes -- wipe ";
        if(which >= 0) {
            confirm = (char*)calloc(
                    strlen(confirm_prefix) +
                    strlen(wipe_partitions[which].name) +
                    strlen(confirm_suffix) + 1, sizeof(char));
            strcpy(confirm, confirm_prefix);
            strcat(confirm, wipe_partitions[which].name);
            strcat(confirm, confirm_suffix);

            confirm_select = (char*)calloc(
                    strlen(confirm_select_prefix) +
                    strlen(wipe_partitions[which].name) + 1, sizeof(char));
            strcpy(confirm_select, confirm_select_prefix);
            strcat(confirm_select, wipe_partitions[which].name);
        } else {
            // we are wiping all
            confirm = strdup("Confirm wipe of EVERYTHING?");
            confirm_select = strdup(" Yes -- wipe EVERYTHING");
        }

        char* headers[] = { confirm,
                            "THIS CAN NOT BE UNDONE.",
                            "", NULL };
        title_headers = prepend_title(headers);

        

        char* items[] = { " No",
                          " No",
                          " No",
                          " No",
                          " No",
                          " No",
                          " No",
                          confirm_select,   // [7]
                          " No",
                          " No",
                          " No",
                          NULL };

        int chosen_item = get_menu_selection(title_headers, items, 1, 0);
        free(confirm);
        free(confirm_select);
        if (chosen_item != 7) {
            return;
        }
    }

    if(which >= 0) {
        ui_print("-- Wiping ");
        ui_print(wipe_partitions[which].name);
        ui_print("... ");
        erase_volume(wipe_partitions[which].path);
        ui_print(" complete.\n");
        if(which == WIPE_PARTITION_DATA && has_datadata()) {
            ui_print("-- Wiping Datadata ... ");
            erase_volume("/datadata");
            ui_print(" complete.\n");
        }
    } else {
        int i;
        for(i = 0; i < num_partitions; ++i) {
            ui_print("-- Wiping ");
            ui_print(wipe_partitions[i].name);
            ui_print("... ");
            erase_volume(wipe_partitions[i].path);
            ui_print("complete.\n");
        }
        if(has_datadata()) {
            ui_print("-- Wiping Datadata ... ");
            erase_volume("/datadata");
            ui_print(" complete.\n");
        }
    }
}

void wipe_batts(int confirm) {
    if (confirm) {
        char** title_headers = NULL;

        if (title_headers == NULL) {
            char* headers[] = { "Confirm wipe of battery stats?",
                                "THIS CAN NOT BE UNDONE.",
                                "",
                                NULL };
            title_headers = prepend_title(headers);
        }

        char* items[] = { " No",
                          " No",
                          " No",
                          " No",
                          " No",
                          " No",
                          " No",
                          " Yes -- Wipe battery stats",   // [7]
                          " No",
                          " No",
                          " No",
                          NULL };

        int chosen_item = get_menu_selection(title_headers, items, 1, 0);
        if (chosen_item != 7) {
            return;
        }
    }

    ui_print("\n-- Wiping battery stats...\n");
    ensure_path_mounted("/data");
    remove("/data/system/batterystats.bin");
    ui_print("\n Battery Statistics cleared.\n");
}

#define ITEM_WIPE_ALL    1
#define ITEM_WIPE_SYSTEM 2
#define ITEM_WIPE_DATA   3
#define ITEM_WIPE_BOOT   4
#define ITEM_WIPE_CACHE  5
#define ITEM_WIPE_MISC   6
#define ITEM_WIPE_BATT   7

int wipe_menu_select(int chosen_item, void* data) {
    switch (chosen_item) {
    case ITEM_WIPE_ALL:
        wipe_partition(-1, ui_text_visible());
        break;
    case ITEM_WIPE_SYSTEM:
        wipe_partition(WIPE_PARTITION_SYSTEM, ui_text_visible());
        break;
    case ITEM_WIPE_DATA:
        wipe_partition(WIPE_PARTITION_DATA, ui_text_visible());
        break;
    case ITEM_WIPE_BOOT:
        wipe_partition(WIPE_PARTITION_BOOT, ui_text_visible());
        break;
    case ITEM_WIPE_CACHE:
        wipe_partition(WIPE_PARTITION_CACHE, ui_text_visible());
        break;
    case ITEM_WIPE_MISC:
        wipe_partition(WIPE_PARTITION_MISC, ui_text_visible());
        break;
    case ITEM_WIPE_BATT:
        wipe_batts(ui_text_visible());
        break;
    }

    return chosen_item;
}

void show_wipe_menu()
{
    recovery_menu_item** items = (recovery_menu_item**)calloc(8, sizeof(recovery_menu_item*));
    items[0] = create_menu_item(ITEM_WIPE_ALL,    "Wipe All");
    items[1] = create_menu_item(ITEM_WIPE_SYSTEM, "Wipe system");
    items[2] = create_menu_item(ITEM_WIPE_DATA,   "Wipe data");
    items[3] = create_menu_item(ITEM_WIPE_BOOT,   "Wipe boot");
    items[4] = create_menu_item(ITEM_WIPE_CACHE,  "Wipe cache");
    items[5] = create_menu_item(ITEM_WIPE_MISC,   "Wipe misc");
    items[6] = create_menu_item(ITEM_WIPE_BATT,   "Wipe battery stats");
    items[7] = NULL;

    char* headers[] = { "Choose an item to wipe",
                        "or press DEL or POWER to return",
                        "USE CAUTION:",
                        "These operations *CANNOT BE UNDONE*",
                        "", NULL };

    recovery_menu* menu = create_menu(
            headers,
            items,
            /* no data */ NULL,
            /* no on_create */ NULL,
            /* no on_create_items */ NULL,
            &wipe_menu_select,
            /* no on_destroy */ NULL);
    display_menu(menu);
    destroy_menu(menu);
}