#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include <stdio.h>

#define LOOP_DELAY_MS           10      // Loop sampling time (ms)
#define DEBOUNCE_TIME           40      // Debounce time (ms)
#define NROWS                   4       // Number of keypad rows
#define NCOLS                   4       // Number of keypad columns

#define ACTIVE                  0       // Keypad active state (0 = low, 1 = high)

#define NOPRESS                 '\0'    // NOPRESS character

int row_pins[] = {GPIO_NUM_18, GPIO_NUM_17, GPIO_NUM_16, GPIO_NUM_15};     // Pin numbers for rows
int col_pins[] = {GPIO_NUM_7, GPIO_NUM_6, GPIO_NUM_5, GPIO_NUM_4};   // Pin numbers for columns

char keypad_array[NROWS][NCOLS] = {   // Keypad layout
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};



void initKeypad(void){
    for (int i = 0; i <= NCOLS - 1; i++){
        int gpioColNum = col_pins[i];
        gpio_reset_pin(gpioColNum);
        gpio_set_direction(gpioColNum, GPIO_MODE_INPUT);
        if (!ACTIVE){
            gpio_pullup_en(gpioColNum);
        }
        else{
            gpio_pulldown_en(gpioColNum);
        }
    }
    for (int i = 0; i <= NROWS - 1; i++){
        int gpioRowNum = row_pins[i];
        gpio_reset_pin(gpioRowNum);
        gpio_set_direction(gpioRowNum, GPIO_MODE_OUTPUT);
        gpio_set_level(gpioRowNum, !ACTIVE);
    }
}
char scanKeypad(void){
    for(int i = 0; i <= NROWS -1; i++){
        gpio_set_level(row_pins[i], ACTIVE);
        for (int j = 0; j <= NCOLS - 1; j++){
            if (gpio_get_level(col_pins[j]) == ACTIVE){
                gpio_set_level(row_pins[i], !ACTIVE);
                return keypad_array[i][j];
            }
        }
        gpio_set_level(row_pins[i], !ACTIVE);
    }
    return NOPRESS;
}


void app_main(void){
    initKeypad();
    typedef enum {waitPress, debounce, waitRelease} State_t;
    State_t State;
    State = waitPress;
    char newKey = NOPRESS;
    bool timedOut = false;
    char lastKey;
    int time = 0;
    while(1){
        newKey = scanKeypad();
        switch (State){
            case waitPress:
                if (newKey != NOPRESS){
                    lastKey = newKey;
                    State = debounce;
                    time = 0;
                }
                break;
            case debounce:
                if (!timedOut){
                    time += LOOP_DELAY_MS;
                    if (time >= DEBOUNCE_TIME){
                        timedOut = true;
                        State = debounce;
                    }
                }
                if (timedOut){
                    if (newKey != lastKey){
                        State = waitPress;
                    }
                    if (newKey == lastKey){
                        State = waitRelease;
                    }
                }
                break;
            case waitRelease:
                if (newKey != NOPRESS){
                    State = waitRelease;
                }
                else{
                    printf("%c\n", lastKey);
                    State = waitPress;
                }
                break;
        }
        vTaskDelay(LOOP_DELAY_MS / portTICK_PERIOD_MS);
    }
    }