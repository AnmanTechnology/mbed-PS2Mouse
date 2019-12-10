#include <mbed.h>
#include "PS2Mouse.h"

#define LED_BUILTIN PC_13

#define SERIAL2_TX PA_2
#define SERIAL2_RX PA_3

Serial serial(SERIAL2_TX, SERIAL2_RX, 9600);
DigitalOut led(LED_BUILTIN);
PS2Mouse ps2ms(PA_6, PA_5); //Clock pin, Data pin

int main()
{
    led = 1;
    wait_ms(2000);

    serial.printf("STM32 bluepill mbed test.\n");
    led = 0;

    if (!ps2ms.begin())
    {
        serial.printf("Can't connect to PS/2 Mouse\n");
        return 0;
    }
    serial.printf("Connected\n");

    while (true)
    {
        ps2ms.update();

        printf("%d\t", ps2ms.info.btnLeft);
        printf("%d\t", ps2ms.info.btnCenter);
        printf("%d\t", ps2ms.info.btnRight);
        printf("%d\t", ps2ms.info.moveX);
        printf("%d\n", ps2ms.info.moveY);

        wait_ms(50);
    }
}