# Opgave 4 — External Interrupt + OLED Display (ATmega328PB)

Det her er min løsning til Embedded Opgave 4.

Programmet tæller tryk på en knap via et eksternt interrupt, sender tælleren ud på begge UART’er og viser den på et SSD1306 OLED-display. Samtidig kører den gamle 1-sekunds timer stadig, og de to UART’er laver bogstaver om til store/små bogstaver.

Jeg bruger **ikke** Kramer-print. Displayet er koblet direkte med ledninger til SDA/SCL.

## Hvad opgaven kræver

1. Eksternt interrupt 0 fra trykknap på **PORTD2**
2. Kun ét tælling per tryk (nedadgående flanke tæller, opadgående tæller ikke)
3. Gyldigt tryk → tæl op og send tallet på **UART0 og UART1**
4. Samme tal skrives også på OLED-displayet
5. Timer-interrupt hvert sekund → skriv sekundværdi på **UART1**
6. UART0 RX → store bogstaver, send tilbage på UART0
7. UART1 RX → små bogstaver, send tilbage på UART1

## Opkobling

### Trykknap (INT0)

| Signal | Board |
|--------|--------|
| Knap til GND | **PD2** (INT0) |
| Intern pull-up | aktiveret i kode |

Når knappen trykkes, går PD2 fra 5 V til 0 V (nedadgående flanke).

### SSD1306 OLED via I2C (ledninger)

| Display | Board |
|---------|--------|
| SDA | **PC4** (SDA0) |
| SCL | **PC5** (SCL0) |
| VCC | 5 V |
| GND | GND |

I2C-adressen er `0x3C`. Jeg bruger de medfølgende driverfiler (`ssd1306` + `twi`).

### UART / CP2102

| UART | Pins | PC-port | Rolle |
|------|------|---------|--------|
| **USART0** | PD0/PD1 | Board-USB | Echo med STORE bogstaver |
| **USART1** | PB3 (TX) / PB4 (RX) | CP2102 | Echo med små bogstaver + sekundtæller |

CP2102:

| CP2102 | Board |
|--------|--------|
| TXD | PB4 |
| RXD | PB3 |
| GND | GND |

## Hvordan jeg har lavet det

### External Interrupt (kun ét tælling per tryk)

Interrupt sættes først til **nedadgående flanke** (`ISC0 = 10`):

```c
EICRA |= (1 << ISC01);
EICRA &= ~(1 << ISC00);
EIMSK |= (1 << INT0);
```

I `INT0`-ISR’en gør jeg det, opgaven foreslår:

1. Ved nedadgående flanke → tæl op, sæt et flag, skift til **opadgående** flanke
2. Ved opadgående flanke → tæl **ikke**, skift tilbage til nedadgående

Selve UART-print og display-opdatering kører i `main`, når flaget er sat. Så holder ISR’en kort.

### Display

Efter hvert gyldigt tryk cleares skærmen, og tælleren skrives med `SSD1306_DrawString`. Init kaldes én gang ved start.

### Timer

`Timer1` kører CTC med callback hvert **1000 ms**. I interrupt toggles LED på PB5 og sættes `second_flag`. Main skriver derefter:

```text
Sekund: N
```

ud på UART1.

### UART case-konvertering

- **UART0 RX-ISR:** bogstaver → STORE, send tilbage på UART0
- **UART1 RX-ISR:** bogstaver → små, send tilbage på UART1

Andre tegn (tal, mellemrum, osv.) sendes uændret tilbage.

## Filer

| Fil | Indhold |
|-----|---------|
| `src/main.c` | Init, main-loop, display-opdatering |
| `src/ext_interrupt.c` | INT0 setup + falling/rising-trick |
| `src/Uart.c` | USART0 + USART1 med RX-interrupts |
| `src/timer1.c` | 1 s timer-callback |
| `src/ssd1306.c` / `src/twi.c` | OLED + I2C-drivere |
| `include/` | Headers + font |

## Test

1. Flash til ATmega328PB Xplained Mini
2. Åbn to serielle porte @ 9600 8N1 (board-USB + CP2102)
3. UART1 skal tick’e `Sekund: 1, 2, 3…`
4. Tryk på knappen → begge UART’er får `External Interrupt: N`, og displayet opdateres
5. Send `a` på UART0 → få `A` tilbage
6. Send `B` på UART1 → få `b` tilbage
7. Ét tryk må kun give én optælling (ikke dobbelt-tælling ved bounce/slip)

Bygget med VS Code + MPLAB / XC8 til ATmega328PB.
