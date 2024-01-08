# Lookup Table Definitions

These are the lookup table definitions for the CCL in the AVR to generate the wait signal.

## LUT 0

| A7 | A6 | IORQ | IOWAIT        |
|----|----|------|---------------|
| 0  | 0  | 0    | 0 = 0x00-0x3F |
| 0  | 0  | 1    | 1             |
| 0  | 1  | 0    | 0 = 0x40-0x7F |
| 0  | 1  | 1    | 1             |
| 1  | 0  | 0    | 0 = 0x80-0xBF |
| 1  | 0  | 1    | 1             |
| 1  | 1  | 0    | 0 = 0xC0-0xFF |
| 1  | 1  | 1    | 1             |

## LUT 5

| M1 | WR | IOWAIT | CYCLE       |
|----|----|--------|-------------|
| 0  | 0  | 0      | 1           |
| 0  | 0  | 1      | 1           |
| 0  | 1  | 0      | 0 = INTACK  |
| 0  | 1  | 1      | 0 = OPFETCH |
| 1  | 0  | 0      | 0 = IOWR    |
| 1  | 0  | 1      | 0 = MEMWR   |
| 1  | 1  | 0      | 0 = IORD    |
| 1  | 1  | 1      | 0 = MEMDR   |

## LUT 4

| MREQ | BUSRQ | CYCLE | WAIT       |
|------|-------|-------|------------|
| 0    | 0     | 0     | 1          |
| 0    | 0     | 1     | 1          |
| 0    | 1     | 0     | 0 = MWAIT  |
| 0    | 1     | 1     | 1          |
| 1    | 0     | 0     | 1          |
| 1    | 0     | 1     | 1          |
| 1    | 1     | 0     | 0 = IOWAIT |
| 1    | 1     | 1     | 1          |
