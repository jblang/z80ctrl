10 REM get values from RTC
15 OUT 0,0: REM select RTC chip
20 OUT 1,6: REM year register
30 YR=INP(2)
40 OUT 1,5: REM month register
50 MO=INP(2)
60 OUT 1,4: REM day register
70 DY=INP(2)
80 OUT 1,2: REM hour register
90 HR=INP(2)
100 OUT 1,1: REM minute register
110 MI=INP(2)
120 OUT 1,0: REM second register
130 S=INP(2)
140 REM handle AM/PM if needed
150 AMPM$=""
160 IF (HR AND &H40)=0 THEN 190: REM skip if time is in 24-hour format
170 IF HR AND &H20 THEN AMPM$="PM" ELSE AMPM$="AM"
180 HR=HR AND &H1F: REM get rid of am/pm and 24 hour bits
190 REM convert values from BCD
200 DEF FNBCD(V)=(V AND &HF0)/16*10+(V AND &HF)
210 YR=FNBCD(YR)+2000
220 MO=FNBCD(MO)
230 DY=FNBCD(DY)
240 HR=FNBCD(HR)
250 MI=FNBCD(MI)
260 S=FNBCD(S)
270 PRINT USING "The time is ##:##:##& on ##/##/####"; HR, MI, S, AMPM$, MO, DY, YR