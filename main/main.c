#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_system.h"

// Direct memory-mapped addresses for ESP8266 Hardware Timer 1 (FRC1)
#define FRC1_LOAD_REG   (*(volatile uint32_t *)0x60000600) // Reload value
#define FRC1_COUNT_REG  (*(volatile uint32_t *)0x60000604) // Current live tick count
#define FRC1_CTRL_REG   (*(volatile uint32_t *)0x60000608) // Control & interrupt status flags

static int count = 0;

void timer_callback(void *arg) {
    count++;

    // 1. Read live hardware timer registers
    uint32_t load_val  = FRC1_LOAD_REG;
    uint32_t curr_tick = FRC1_COUNT_REG;
    uint32_t ctrl_reg  = FRC1_CTRL_REG;

    // Extract specific status/flag bits from FRC1_CTRL_REG
    uint8_t timer_enabled = (ctrl_reg >> 7) & 0x01; // Bit 7: Timer Enable flag
    uint8_t int_status    = (ctrl_reg >> 8) & 0x01; // Bit 8: Interrupt pending flag

    // 2. Read RAM (Heap) usage
    uint32_t free_ram     = esp_get_free_heap_size();
    uint32_t min_free_ram = esp_get_minimum_free_heap_size();

    printf("\n--- [TIMER TRIGGER #%d] ---\n", count);
    printf("Hardware Registers | LOAD: 0x%08X | COUNT: 0x%08X | CTRL: 0x%08X\n",
           load_val, curr_tick, ctrl_reg);
    printf("Timer Flags        | Enabled: %u | Interrupt Flag: %u\n",
           timer_enabled, int_status);
    printf("RAM (Heap) Status  | Free Now: %u bytes | Lowest Ever: %u bytes\n",
           free_ram, min_free_ram);
}

void app_main(void) {
    const esp_timer_create_args_t timer_args = {
        .callback = &timer_callback,
        .name = "ten_sec_timer"
    };

    esp_timer_handle_t periodic_timer;
    esp_timer_create(&timer_args, &periodic_timer);

    // 10,000,000 microseconds = 10 seconds
    esp_timer_start_periodic(periodic_timer, 10000000);
    printf("10-second diagnostic timer started!\n");
}
