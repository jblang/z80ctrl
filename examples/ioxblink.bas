10 OUT 0, 1: REM select chip address 1
20 OUT 1, 0: REM select register IODIRA0 (0x00)
30 OUT 2, 0: REM set register value to 0 (all pins outputs)
40 OUT 1, &H12: REM select register GPIOA0 (0x12)
50 FOR I = 1 TO 10
60 OUT 2, 0: REM turn all pins on GPIOA off
70 FOR J = 1 TO 1000:NEXT
80 OUT 2, &HFF: REM turn all pins on GPIOA on
90 FOR J = 1 TO 1000:NEXT
100 NEXT