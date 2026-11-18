#include "../include/ProjectDefines.h"
#include "../include/Uart.h"
#include "../include/timer1.h"
#include "../include/ext_interrupt.h"
#include "../include/ssd1306.h"

#include <stdio.h>
#include <string.h>

static volatile bool second_flag = false;
static uint32_t seconds_since_start = 0;
static uint16_t last_ext_count = 0;

static void on_second_tick(void)
{
    PORTB ^= (1 << RedLedBit);
    second_flag = true;
}

static void display_show_count(uint16_t count)
{
    char line[24];

    SSD1306_ClearScreen();
    SSD1306_SetPosition(0, 0);
    SSD1306_DrawString("Opgave 4", NORMAL);
    SSD1306_SetPosition(0, 2);
    SSD1306_DrawString("Ext INT count:", NORMAL);

    snprintf(line, sizeof(line), "%u", (unsigned)count);
    SSD1306_SetPosition(0, 4);
    SSD1306_DrawString(line, BOLD);
}

static void report_ext_count(uint16_t count)
{
    char line[48];

    snprintf(line, sizeof(line), "External Interrupt: %u\r\n", (unsigned)count);
    uart_print_both(line);
    display_show_count(count);
}

int main(void)
{
    uint16_t count;

    DDRB |= (1 << RedLedBit);
    PORTB |= (1 << RedLedBit);

    RS232Init();
    ext_interrupt_init();
    timer1_start_ms(1000, on_second_tick);

    /* Display via I2C (SDA=PC4, SCL=PC5) — direkte ledninger, ikke Kramer-print */
    if (SSD1306_Init(SSD1306_ADDR) == SSD1306_SUCCESS) {
        display_show_count(0);
    } else {
        uart0_print("SSD1306 init fejlede\r\n");
    }

    sei();

    uart0_print("\r\n=== Opgave 4: Ext Interrupt + Display ===\r\n");
    uart0_print("UART0: store bogstaver echo\r\n");
    uart1_print("\r\nUART1: smaa bogstaver echo + sekundtaeller\r\n");
    uart_print_both("Tryk paa knappen paa PD2 for at taelle.\r\n");

    for (;;) {
        if (second_flag) {
            second_flag = false;
            seconds_since_start++;
            fprintf(uart1_stream(),
                    "Sekund: %lu\r\n",
                    (unsigned long)seconds_since_start);
        }

        if (ext_interrupt_take_event(&count)) {
            last_ext_count = count;
            report_ext_count(last_ext_count);
        }
    }
}
