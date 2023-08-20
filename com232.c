/*
  Copyright (C) 2006 Marcin Slonicki <marcin@softservice.com.pl>.

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software Foundation,
  Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

 -----------------------------------------------------------------------
        this file is a part of the TOYOTA Corolla MP3 Player Project
 -----------------------------------------------------------------------
                http://www.softservice.com.pl/corolla/avc

 May 28 / 2009	- version 2

*/

#include "com232.h"
#include <avr/interrupt.h>
#include <avr/io.h>

byte RS232_RxCharBuffer[25], RS232_RxCharBegin, RS232_RxCharEnd;
byte readkey;

void RS232_Init(void) {
  RS232_RxCharBegin = RS232_RxCharEnd = 0;

  USART0.CTRLA = USART_RXCIE_bm; // Enable receive interrupts
  USART0.CTRLB = USART_RXEN_bm | USART_TXEN_bm |
                 USART_RXMODE_NORMAL_gc; // Enable Rx/Tx and set receive mode
                                         // normal
  USART0.CTRLC = USART_CMODE_ASYNCHRONOUS_gc | USART_PMODE_DISABLED_gc |
                 USART_CHSIZE_8BIT_gc |
                 USART_SBMODE_1BIT_gc; // Async UART with 8N1 config
  USART0.BAUD = 256; // 250k baud rate (64*F_CPU/(16*250k)) for F_CPU = 16MHz
}

ISR(USART0_RXC_vect) {
  RS232_RxCharBuffer[RS232_RxCharEnd] = USART0_RXDATAL; // Store received
                                                        // character to the End
                                                        // of Buffer
  RS232_RxCharEnd++;
}

void RS232_SendByte(byte Data) {
  loop_until_bit_is_set(USART0_STATUS,
                        USART_DREIF_bp); // wait for UART to become available
  USART0_TXDATAL = Data;                 // send character
}

void RS232_Print(const char *pBuf) {
  register byte c;
  while ((c = *pBuf++)) {
    if (c == '\n')
      RS232_SendByte('\r');
    RS232_SendByte(c);
  }
}

void RS232_PrintHex4(byte Data) {
  byte Character = Data & 0x0f;
  Character += '0';
  if (Character > '9')
    Character += 'A' - '0' - 10;
  RS232_SendByte(Character);
}

void RS232_PrintHex8(byte Data) {
  RS232_PrintHex4(Data >> 4);
  RS232_PrintHex4(Data);
}

void RS232_PrintDec(byte Data) {
  if (Data > 99) {
    RS232_SendByte('*');
    return;
  }
  if (Data < 10) {
    RS232_SendByte('0' + Data);
    return;
  }
  byte c;
  unsigned short v, v1;
  v = Data;
  v1 = v / 10;
  c = '0' + (v - v1 * 10);
  RS232_SendByte('0' + v1);
  RS232_SendByte(c);
}

void RS232_PrintDec2(byte Data) {
  if (Data < 10)
    RS232_SendByte('0');
  RS232_PrintDec(Data);
}

char *itoa(int i, char b[]) {
  char const digit[] = "0123456789";
  char *p = b;
  if (i < 0) {
    *p++ = '-';
    i *= -1;
  }
  int shifter = i;
  do { // Move to where representation ends
    ++p;
    shifter = shifter / 10;
  } while (shifter);
  *p = '\0';
  do { // Move back, inserting digits as u go
    *--p = digit[i % 10];
    i = i / 10;
  } while (i);
  return b;
}
