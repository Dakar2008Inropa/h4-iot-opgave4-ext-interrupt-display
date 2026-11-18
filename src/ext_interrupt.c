#include "../include/ext_interrupt.h"

/*
 * Trykknap på PORTD2 / INT0.
 *
 * Hint fra opgaven: tæl kun på nedadgående flanke (tryk).
 * Skift derefter til opadgående flanke (slip) uden at tælle,
 * og skift tilbage til nedadgående igen.
 */

static volatile uint16_t ext_count = 0;
static volatile bool event_pending = false;
static volatile bool wait_falling = true;

void ext_interrupt_init(void)
{
    /* PD2 som input med intern pull-up */
    DDRD &= (uint8_t)~(1 << DDD2);
    PORTD |= (1 << PORTD2);

    /* ISC0 = 10 => interrupt på nedadgående flanke */
    EICRA |= (1 << ISC01);
    EICRA &= (uint8_t)~(1 << ISC00);

    /* Enable External Interrupt Request 0 */
    EIMSK |= (1 << INT0);

    wait_falling = true;
    ext_count = 0;
    event_pending = false;
}

bool ext_interrupt_take_event(uint16_t *count_out)
{
    bool had_event;

    if (count_out == NULL) {
        return false;
    }

    cli();
    had_event = event_pending;
    if (had_event) {
        *count_out = ext_count;
        event_pending = false;
    }
    sei();

    return had_event;
}

ISR(INT0_vect)
{
    if (wait_falling) {
        /* Gyldigt tryk — tæl op og vent på slip */
        ext_count++;
        event_pending = true;

        /* ISC0 = 11 => opadgående flanke */
        EICRA |= (1 << ISC01) | (1 << ISC00);
        wait_falling = false;
    } else {
        /* Slip — tæl ikke, men gør klar til næste tryk */
        EICRA |= (1 << ISC01);
        EICRA &= (uint8_t)~(1 << ISC00);
        wait_falling = true;
    }
}
