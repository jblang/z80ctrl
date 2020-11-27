/*
 * Copyright 2001-2010 Georges Menie (www.menie.org)
 * All rights reserved.
 * 
 * YMODEM support added by J.B. Langston
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the University of California, Berkeley nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
Reference: http://pauillac.inria.fr/~doligez/zmodem/ymodem.txt

           Figure 4.  YMODEM Batch Transmission Session (2 files)

           SENDER                                  RECEIVER
                                                   "sb foo.c baz.c<CR>"
           "sending in batch mode etc."
                                                   C (command:rb)
           SOH 00 FF foo.c NUL[123] CRC CRC
                                                   ACK
                                                   C
           SOH 01 FE Data[128] CRC CRC
                                                   ACK
           SOH 02 FC Data[128] CRC CRC
                                                   ACK
           SOH 03 FB Data[100] CPMEOF[28] CRC CRC
                                                   ACK
           EOT
                                                   NAK
           EOT
                                                   ACK
                                                   C
           SOH 00 FF baz.c NUL[123] CRC CRC
                                                   ACK
                                                   C
           SOH 01 FB Data[100] CPMEOF[28] CRC CRC
                                                   ACK
           EOT
                                                   NAK
           EOT
                                                   ACK
                                                   C
           SOH 00 FF NUL[128] CRC CRC
                                                   ACK
 */

#include "ff.h"
#include "uart.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <util/delay.h>
#include <util/crc16.h>
#include <avr/pgmspace.h>

#define SOH 0x01
#define STX 0x02
#define EOT 0x04
#define ACK 0x06
#define NAK 0x15
#define CAN 0x18
#define CTRLZ 0x1A

#define MAXRETRANS 25
#define TRANSMIT_XMODEM_1K

int inbyte(uint16_t timeout) // msec timeout
{
    uint16_t c;
    while (uart_testrx(0) == 0) {
        _delay_ms(1);
        if (timeout) {
            if (--timeout == 0)
                return -2;
        }
    }

    return uart_getc(0);
}

void outbyte(char c) {
    uart_putc(0, c); 
}

uint16_t crc16(const uint8_t *buf, int len)
{
    uint16_t crc = 0;
    while (len--)
        crc = _crc_xmodem_update(crc, *buf++);
    return crc;
}

static int check(int crc, const uint8_t *buf, int sz)
{
    if (crc) {
        uint16_t crc = crc16(buf, sz);
        uint16_t tcrc = (buf[sz] << 8) + buf[sz + 1];
        if (crc == tcrc)
            return 1;
    } else {
        int i;
        uint8_t cks = 0;
        for (i = 0; i < sz; ++i) {
            cks += buf[i];
        }
        if (cks == buf[sz])
            return 1;
    }

    return 0;
}

static void flushinput(void)
{
    while (inbyte(1000) >= 0)
        ;
}

static void cancel(void)
{
        flushinput();
        outbyte(CAN);
        outbyte(CAN);
        outbyte(CAN);
}

int xm_receive(int argc, char *argv[])
{
    uint8_t xbuff[1030]; /* 1024 for XModem 1k + 3 head chars + 2 crc + nul */
    uint8_t *p;
    int bufsz, crc = 0;
    uint8_t trychar = 'C';
    uint8_t packetno = 1;
    int i, c, len = 0;
    int retry, retrans = MAXRETRANS;

    FIL fil;
    FRESULT fr;
    UINT bw;
    int curfile = 0;
    char filename[255] = "NONAME";
    int size = 0;
    uint32_t mtime = 0;
    uint8_t ymodem = 0;

    flushinput();
    for (;;) {
        for (retry = 0; retry < 16; ++retry) {
            if (trychar)
                outbyte(trychar);
            if ((c = inbyte(10000)) >= 0) {
                switch (c) {
                    case SOH:
                        bufsz = 128;
                        goto start_recv;
                    case STX:
                        bufsz = 1024;
                        goto start_recv;
                    case EOT:
                        // done with current file
                        if (f_error(&fil) == FR_OK) {
                            // ymodem size was specified, remove extra bytes
                            if (size > 0) {
                                f_lseek(&fil, size);
                                f_truncate(&fil);
                            }
                            f_close(&fil);
                        }
                        if (ymodem == 0) {
                            // xmodem EOT; ack and return
                            flushinput();
                            outbyte(ACK);
                            return len;
                        } else if (ymodem == 1) {
                            // ymodem first EOT; nak and prepare to ack second EOT
                            outbyte(NAK);
                            ymodem = 2;
                            retry = 0;
                            continue;
                        } else if (ymodem == 2) {
                            // ymodem second EOT; ack and wait for next file
                            outbyte(ACK);
                            trychar = 'C';
                            ymodem = 0;
                            packetno = 1;
                            retry = 0;
                            continue;
                        }
                        break;
                    case CAN:
                        if ((c = inbyte(1000)) == CAN) {
                            flushinput();
                            outbyte(ACK);
                            return -1; /* canceled by remote */
                        }
                        break;
                    default:
                        flushinput();
                        break;
                }
            }
        }
        if (trychar == 'C') {
            trychar = NAK;
            continue;
        }
        cancel();
        return -2; /* sync error */

    start_recv:
        if (trychar == 'C')
            crc = 1;
        trychar = 0;
        p = xbuff;
        *p++ = c;
        for (i = 0; i < (bufsz + (crc ? 1 : 0) + 3); ++i) {
            if ((c = inbyte(1000)) < 0)
                goto reject;
            *p++ = c;
        }

        if (xbuff[1] == (uint8_t)(~xbuff[2]) &&
            (xbuff[1] == packetno || xbuff[1] == (uint8_t)packetno - 1) &&
            check(crc, &xbuff[3], bufsz)) {
            if (xbuff[1] == 0) {
                // ymodem metadata packet
                if (xbuff[3] == 0) {
                    // empty filename; ymodem session complete
                    flushinput();
                    outbyte(ACK);
                    return len;
                }
                // SOH 00 FF filename NUL size space mtime NUL[...] CRC CRC
                strncpy(filename, &xbuff[3], sizeof(filename));
                i = strlen(filename) + 4;
                i = sscanf_P(&xbuff[i], PSTR("%d %o"), &size, &mtime);
                if (i == 0)
                    size = 0;
                if (i < 2)
                    mtime = 0;
                // get ready for data packet
                outbyte(ACK);
                trychar = 'C';
                ymodem = 1;
                continue;
            }
            if (xbuff[1] == packetno) {
                if (packetno == 1) {
                    // first data packet; open new file
                    if (curfile < argc)     // override filename if given locally
                        strncpy(filename, argv[curfile], 255);
                    curfile++;
                    if (f_open(&fil, filename, FA_WRITE | FA_CREATE_ALWAYS) != FR_OK) {
                        cancel();
                        return -4;
                    }
                }
                if (f_write(&fil, &xbuff[3], bufsz, &bw) != FR_OK) {
                    cancel();
                    return -4;
                }
                len += bw;
                ++packetno;
                retrans = MAXRETRANS + 1;
            }
            if (--retrans <= 0) {
                flushinput();
                cancel();
                return -3; // too many retry error
            }
            outbyte(ACK);
            continue;
        }
    reject:
        flushinput();
        outbyte(NAK);
    }
}

int xm_transmit(FIL *file)
{
    uint8_t
        xbuff[1030]; /* 1024 for XModem 1k + 3 head chars + 2 crc + nul */
    int bufsz, crc = -1;
    uint8_t packetno = 1;
    int i, c, len = 0;
    int retry;

    for (;;) {
        for (retry = 0; retry < 16; ++retry) {
            if ((c = inbyte(10000)) >= 0) {
                switch (c) {
                    case 'C':
                        crc = 1;
                        goto start_trans;
                    case NAK:
                        crc = 0;
                        goto start_trans;
                    case CAN:
                        if ((c = inbyte(1000)) == CAN) {
                            outbyte(ACK);
                            flushinput();
                            return -1; /* canceled by remote */
                        }
                        break;
                    default:
                        break;
                }
            }
        }
        outbyte(CAN);
        outbyte(CAN);
        outbyte(CAN);
        flushinput();
        return -2; /* no sync */

        for (;;) {
        start_trans:
#ifdef TRANSMIT_XMODEM_1K
            xbuff[0] = STX;
            bufsz = 1024;
#else
            xbuff[0] = SOH;
            bufsz = 128;
#endif
            xbuff[1] = packetno;
            xbuff[2] = ~packetno;
            memset(&xbuff[3], 0, bufsz);
            f_read(file, &xbuff[3], bufsz, &c);
            if (c > bufsz)
                c = bufsz;
            if (c > 0) {
                if (c < bufsz)
                    xbuff[3 + c] = CTRLZ;
                if (crc) {
                    uint16_t ccrc = crc16(&xbuff[3], bufsz);
                    xbuff[bufsz + 3] = (ccrc >> 8) & 0xFF;
                    xbuff[bufsz + 4] = ccrc & 0xFF;
                } else {
                    uint8_t ccks = 0;
                    for (i = 3; i < bufsz + 3; ++i) {
                        ccks += xbuff[i];
                    }
                    xbuff[bufsz + 3] = ccks;
                }
                for (retry = 0; retry < MAXRETRANS; ++retry) {
                    for (i = 0; i < bufsz + 4 + (crc ? 1 : 0); ++i) {
                        outbyte(xbuff[i]);
                    }
                    if ((c = inbyte(1000)) >= 0) {
                        switch (c) {
                            case ACK:
                                ++packetno;
                                len += bufsz;
                                goto start_trans;
                            case CAN:
                                if ((c = inbyte(1000)) == CAN) {
                                    outbyte(ACK);
                                    flushinput();
                                    return -1; /* canceled by remote */
                                }
                                break;
                            case NAK:
                            default:
                                break;
                        }
                    }
                }
                outbyte(CAN);
                outbyte(CAN);
                outbyte(CAN);
                flushinput();
                return -4; /* xmit error */
            } else {
                for (retry = 0; retry < 10; ++retry) {
                    outbyte(EOT);
                    if ((c = inbyte((1000) << 1)) == ACK)
                        break;
                }
                flushinput();
                return (c == ACK) ? len : -5;
            }
        }
    }
}