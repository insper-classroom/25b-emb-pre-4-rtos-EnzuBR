#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

const int BTN_PIN_R = 28;
const int BTN_PIN_Y = 21;
const int LED_PIN_R = 5;
const int LED_PIN_Y = 10;

QueueHandle_t xQueueBtn;
SemaphoreHandle_t xSemaphoreLedR;
SemaphoreHandle_t xSemaphoreLedY;

void btn_callback(uint gpio, uint32_t events) {
    if (events == GPIO_IRQ_EDGE_FALL) {
        xQueueSendFromISR(xQueueBtn, &gpio, NULL);
    }
}

void led_task_r(void *p) {
    gpio_init(LED_PIN_R);
    gpio_set_dir(LED_PIN_R, GPIO_OUT);

    int state = 0;
    int gpio_id;

    while (true) {
        if (xQueueReceive(xQueueBtn, &gpio_id, 0)) {
            if (gpio_id == BTN_PIN_R) {
                xSemaphoreGive(xSemaphoreLedR);
            }
        }

        if (xSemaphoreTake(xSemaphoreLedR, 0) == pdTRUE) {
            state = !state;
        }

        if (state) {
            gpio_put(LED_PIN_R, 1);
            vTaskDelay(pdMS_TO_TICKS(100));
            gpio_put(LED_PIN_R, 0);
            vTaskDelay(pdMS_TO_TICKS(100));
        } else {
            gpio_put(LED_PIN_R, 0);
            vTaskDelay(pdMS_TO_TICKS(50));
        }
    }
}

void led_task_y(void *p) {
    gpio_init(LED_PIN_Y);
    gpio_set_dir(LED_PIN_Y, GPIO_OUT);

    int state = 0;
    int gpio_id;

    while (true) {
        if (xQueueReceive(xQueueBtn, &gpio_id, 0)) {
            if (gpio_id == BTN_PIN_Y) {
                xSemaphoreGive(xSemaphoreLedY);
            }
        }

        if (xSemaphoreTake(xSemaphoreLedY, 0) == pdTRUE) {
            state = !state;
        }

        if (state) {
            gpio_put(LED_PIN_Y, 1);
            vTaskDelay(pdMS_TO_TICKS(100));
            gpio_put(LED_PIN_Y, 0);
            vTaskDelay(pdMS_TO_TICKS(100));
        } else {
            gpio_put(LED_PIN_Y, 0);
            vTaskDelay(pdMS_TO_TICKS(50));
        }
    }
}

int main() {
    stdio_init_all();

    xQueueBtn = xQueueCreate(10, sizeof(int));
    xSemaphoreLedR = xSemaphoreCreateBinary();
    xSemaphoreLedY = xSemaphoreCreateBinary();

    gpio_init(BTN_PIN_R);
    gpio_set_dir(BTN_PIN_R, GPIO_IN);
    gpio_pull_up(BTN_PIN_R);
    gpio_set_irq_enabled_with_callback(BTN_PIN_R, GPIO_IRQ_EDGE_FALL, true, &btn_callback);

    gpio_init(BTN_PIN_Y);
    gpio_set_dir(BTN_PIN_Y, GPIO_IN);
    gpio_pull_up(BTN_PIN_Y);
    gpio_set_irq_enabled(BTN_PIN_Y, GPIO_IRQ_EDGE_FALL, true);

    xTaskCreate(led_task_r, "LED_R", 256, NULL, 1, NULL);
    xTaskCreate(led_task_y, "LED_Y", 256, NULL, 1, NULL);

    vTaskStartScheduler();

    while (1) {}
    return 0;
}
