EESchema Schematic File Version 4
LIBS:z80ctrl-iox-cache
EELAYER 26 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title "z80ctrl I/O Expander board"
Date "2018-10-25"
Rev "REV1"
Comp "J.B. Langston"
Comment1 "https://github.com/jblang/z80ctrl"
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L Connector_Generic:Conn_01x39 J1
U 1 1 57B2D86C
P 1050 5450
F 0 "J1" H 1050 7500 50  0000 C CNN
F 1 "BUS" V 1150 5450 50  0001 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x39_P2.54mm_Vertical" H 1050 5450 50  0001 C CNN
F 3 "" H 1050 5450 50  0001 C CNN
F 4 "A-638" H 1050 5450 60  0001 C CNN "TaydaPN"
F 5 "s1012ec-39-nd" H 1050 5450 60  0001 C CNN "DigikeyPN"
	1    1050 5450
	-1   0    0    -1  
$EndComp
$Comp
L power:GND #PWR022
U 1 1 592836F7
P 1800 4900
F 0 "#PWR022" H 1800 4650 50  0001 C CNN
F 1 "GND" H 1800 4750 50  0000 C CNN
F 2 "" H 1800 4900 50  0001 C CNN
F 3 "" H 1800 4900 50  0001 C CNN
	1    1800 4900
	-1   0    0    -1  
$EndComp
$Comp
L power:VCC #PWR023
U 1 1 59283729
P 1800 5450
F 0 "#PWR023" H 1800 5300 50  0001 C CNN
F 1 "VCC" H 1800 5600 50  0000 C CNN
F 2 "" H 1800 5450 50  0001 C CNN
F 3 "" H 1800 5450 50  0001 C CNN
	1    1800 5450
	-1   0    0    -1  
$EndComp
NoConn ~ 2000 6750
NoConn ~ 2000 6650
NoConn ~ 2000 6550
NoConn ~ 2000 6450
NoConn ~ 2000 6350
NoConn ~ 2000 6250
NoConn ~ 2000 6150
NoConn ~ 2000 5450
NoConn ~ 2000 5550
$Comp
L power:VCC #PWR029
U 1 1 5A850097
P 10300 900
F 0 "#PWR029" H 10300 750 50  0001 C CNN
F 1 "VCC" H 10300 1050 50  0000 C CNN
F 2 "" H 10300 900 50  0001 C CNN
F 3 "" H 10300 900 50  0001 C CNN
	1    10300 900 
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR030
U 1 1 5A8500E5
P 10750 950
F 0 "#PWR030" H 10750 700 50  0001 C CNN
F 1 "GND" H 10750 800 50  0000 C CNN
F 2 "" H 10750 950 50  0001 C CNN
F 3 "" H 10750 950 50  0001 C CNN
	1    10750 950 
	1    0    0    -1  
$EndComp
$Comp
L power:PWR_FLAG #FLG031
U 1 1 5A850133
P 10750 900
F 0 "#FLG031" H 10750 975 50  0001 C CNN
F 1 "PWR_FLAG" H 10750 1050 50  0000 C CNN
F 2 "" H 10750 900 50  0001 C CNN
F 3 "" H 10750 900 50  0001 C CNN
	1    10750 900 
	1    0    0    -1  
$EndComp
$Comp
L power:PWR_FLAG #FLG032
U 1 1 5A8502AA
P 10300 950
F 0 "#FLG032" H 10300 1025 50  0001 C CNN
F 1 "PWR_FLAG" H 10300 1100 50  0000 C CNN
F 2 "" H 10300 950 50  0001 C CNN
F 3 "" H 10300 950 50  0001 C CNN
	1    10300 950 
	-1   0    0    1   
$EndComp
$Comp
L Connector_Generic:Conn_01x23 J2
U 1 1 57B2E338
P 2200 6250
F 0 "J2" H 2200 6800 50  0000 C CNN
F 1 "BUSEXT" V 2300 6250 50  0001 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x23_P2.54mm_Vertical" H 2200 6250 50  0001 C CNN
F 3 "" H 2200 6250 50  0001 C CNN
F 4 "A-638" H 2200 6250 60  0001 C CNN "TaydaPN"
F 5 "s1012ec-39-nd" H 2200 6250 60  0001 C CNN "DigikeyPN"
	1    2200 6250
	1    0    0    -1  
$EndComp
Wire Wire Line
	10750 900  10750 950 
Wire Wire Line
	10300 950  10300 900 
Wire Wire Line
	1800 4900 1700 4900
Wire Wire Line
	1700 4900 1700 5150
Wire Wire Line
	1700 5150 1250 5150
Wire Wire Line
	1800 5450 1700 5450
Wire Wire Line
	1700 5450 1700 5250
Wire Wire Line
	1700 5250 1250 5250
NoConn ~ 1250 6250
NoConn ~ 1250 6150
NoConn ~ 2000 5350
NoConn ~ 2000 7050
NoConn ~ 2000 6950
NoConn ~ 1250 7050
NoConn ~ 1250 6950
NoConn ~ 1250 6850
NoConn ~ 1250 6750
NoConn ~ 1250 6650
NoConn ~ 1250 6550
NoConn ~ 1250 6450
NoConn ~ 1250 6350
NoConn ~ 1250 4250
NoConn ~ 1250 4150
NoConn ~ 1250 4050
NoConn ~ 1250 3950
NoConn ~ 1250 3850
NoConn ~ 1250 3750
NoConn ~ 1250 3650
NoConn ~ 1250 3550
NoConn ~ 1250 6050
NoConn ~ 1250 5950
NoConn ~ 1250 5850
NoConn ~ 1250 5750
NoConn ~ 1250 5650
NoConn ~ 1250 5550
NoConn ~ 1250 5450
NoConn ~ 1250 5350
NoConn ~ 2000 6050
NoConn ~ 2000 5950
NoConn ~ 2000 5850
NoConn ~ 2000 5750
NoConn ~ 2000 5650
NoConn ~ 1250 5050
NoConn ~ 1250 4950
NoConn ~ 1250 4850
NoConn ~ 1250 4750
NoConn ~ 1250 4650
NoConn ~ 1250 4550
NoConn ~ 1250 4450
NoConn ~ 1250 4350
$Comp
L z80ctrl-iox:MCP23S17 U2
U 1 1 5BD2A51A
P 5650 2050
F 0 "U2" H 5550 3075 50  0000 R CNN
F 1 "MCP23S17" H 5550 3000 50  0000 R CNN
F 2 "Package_DIP:DIP-28_W7.62mm_Socket" H 5700 1100 50  0001 L CNN
F 3 "" H 5900 3050 50  0001 C CNN
F 4 "MCP23S17-E/SP-ND" H 5650 2050 60  0001 C CNN "DigikeyPN"
F 5 "579-MCP23S17-E/SP" H 5650 2050 60  0001 C CNN "MouserPN"
	1    5650 2050
	1    0    0    -1  
$EndComp
$Comp
L Device:C C2
U 1 1 5BD2A520
P 6850 1150
F 0 "C2" H 6875 1250 50  0000 L CNN
F 1 ".1uf" H 6875 1050 50  0000 L CNN
F 2 "Capacitor_THT:C_Disc_D4.7mm_W2.5mm_P5.00mm" H 6888 1000 50  0001 C CNN
F 3 "" H 6850 1150 50  0001 C CNN
	1    6850 1150
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0101
U 1 1 5BD2A526
P 5650 3200
F 0 "#PWR0101" H 5650 2950 50  0001 C CNN
F 1 "GND" H 5650 3050 50  0000 C CNN
F 2 "" H 5650 3200 50  0001 C CNN
F 3 "" H 5650 3200 50  0001 C CNN
	1    5650 3200
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0102
U 1 1 5BD2A52C
P 6850 1300
F 0 "#PWR0102" H 6850 1050 50  0001 C CNN
F 1 "GND" H 6850 1150 50  0000 C CNN
F 2 "" H 6850 1300 50  0001 C CNN
F 3 "" H 6850 1300 50  0001 C CNN
	1    6850 1300
	1    0    0    -1  
$EndComp
NoConn ~ 6150 1650
$Comp
L power:VCC #PWR0103
U 1 1 5BD2A533
P 5650 950
F 0 "#PWR0103" H 5650 800 50  0001 C CNN
F 1 "VCC" H 5650 1100 50  0000 C CNN
F 2 "" H 5650 950 50  0001 C CNN
F 3 "" H 5650 950 50  0001 C CNN
	1    5650 950 
	1    0    0    -1  
$EndComp
NoConn ~ 6150 1750
Connection ~ 5650 1000
Wire Wire Line
	5650 1000 6650 1000
Wire Wire Line
	5650 950  5650 1000
Wire Wire Line
	5650 1000 5650 1050
Wire Wire Line
	5650 3050 5650 3100
$Comp
L Device:Jumper_NC_Dual JP1
U 1 1 5BD2A79B
P 6900 2450
F 0 "JP1" H 6900 2689 50  0000 C CNN
F 1 "A0" H 6900 2598 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x03_P2.54mm_Vertical" H 6900 2450 50  0001 C CNN
F 3 "" H 6900 2450 50  0001 C CNN
	1    6900 2450
	1    0    0    -1  
$EndComp
$Comp
L Device:Jumper_NC_Dual JP3
U 1 1 5BD2A851
P 8200 2450
F 0 "JP3" H 8200 2689 50  0000 C CNN
F 1 "A2" H 8200 2598 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x03_P2.54mm_Vertical" H 8200 2450 50  0001 C CNN
F 3 "" H 8200 2450 50  0001 C CNN
	1    8200 2450
	1    0    0    -1  
$EndComp
$Comp
L Device:Jumper_NC_Dual JP5
U 1 1 5BD2A879
P 7550 5350
F 0 "JP5" H 7550 5589 50  0000 C CNN
F 1 "A0" H 7550 5498 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x03_P2.54mm_Vertical" H 7550 5350 50  0001 C CNN
F 3 "" H 7550 5350 50  0001 C CNN
	1    7550 5350
	1    0    0    -1  
$EndComp
Wire Wire Line
	6900 2550 6900 2650
Wire Wire Line
	6150 2650 6900 2650
Wire Wire Line
	7550 2550 7550 2750
Wire Wire Line
	6150 2750 7550 2750
Wire Wire Line
	8200 2850 8200 2550
Wire Wire Line
	6150 2850 8200 2850
Wire Wire Line
	6650 2450 6650 2050
Wire Wire Line
	6650 2050 7300 2050
Wire Wire Line
	7300 2050 7300 2450
Wire Wire Line
	7300 2050 7950 2050
Wire Wire Line
	7950 2050 7950 2450
Connection ~ 7300 2050
Wire Wire Line
	6650 2050 6650 1000
Connection ~ 6650 2050
Connection ~ 6650 1000
Wire Wire Line
	6650 1000 6850 1000
Wire Wire Line
	7150 2450 7150 2600
Wire Wire Line
	7150 2600 7800 2600
Wire Wire Line
	7800 2600 7800 2450
Wire Wire Line
	7800 2600 8450 2600
Wire Wire Line
	8450 2600 8450 2450
Connection ~ 7800 2600
Wire Wire Line
	7150 2600 7150 3100
Wire Wire Line
	7150 3100 5650 3100
Connection ~ 7150 2600
Connection ~ 5650 3100
Wire Wire Line
	5650 3100 5650 3200
$Comp
L z80ctrl-iox:MCP23S17 U3
U 1 1 5BD2D7B7
P 5650 4950
F 0 "U3" H 5550 5975 50  0000 R CNN
F 1 "MCP23S17" H 5550 5900 50  0000 R CNN
F 2 "Package_DIP:DIP-28_W7.62mm_Socket" H 5700 4000 50  0001 L CNN
F 3 "" H 5900 5950 50  0001 C CNN
F 4 "MCP23S17-E/SP-ND" H 5650 4950 60  0001 C CNN "DigikeyPN"
F 5 "579-MCP23S17-E/SP" H 5650 4950 60  0001 C CNN "MouserPN"
	1    5650 4950
	1    0    0    -1  
$EndComp
$Comp
L Device:C C3
U 1 1 5BD2D7BD
P 6850 4050
F 0 "C3" H 6875 4150 50  0000 L CNN
F 1 ".1uf" H 6875 3950 50  0000 L CNN
F 2 "Capacitor_THT:C_Disc_D4.7mm_W2.5mm_P5.00mm" H 6888 3900 50  0001 C CNN
F 3 "" H 6850 4050 50  0001 C CNN
	1    6850 4050
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0104
U 1 1 5BD2D7C3
P 5650 6100
F 0 "#PWR0104" H 5650 5850 50  0001 C CNN
F 1 "GND" H 5650 5950 50  0000 C CNN
F 2 "" H 5650 6100 50  0001 C CNN
F 3 "" H 5650 6100 50  0001 C CNN
	1    5650 6100
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0105
U 1 1 5BD2D7C9
P 6850 4200
F 0 "#PWR0105" H 6850 3950 50  0001 C CNN
F 1 "GND" H 6850 4050 50  0000 C CNN
F 2 "" H 6850 4200 50  0001 C CNN
F 3 "" H 6850 4200 50  0001 C CNN
	1    6850 4200
	1    0    0    -1  
$EndComp
NoConn ~ 6150 4550
$Comp
L power:VCC #PWR0106
U 1 1 5BD2D7D0
P 5650 3850
F 0 "#PWR0106" H 5650 3700 50  0001 C CNN
F 1 "VCC" H 5650 4000 50  0000 C CNN
F 2 "" H 5650 3850 50  0001 C CNN
F 3 "" H 5650 3850 50  0001 C CNN
	1    5650 3850
	1    0    0    -1  
$EndComp
NoConn ~ 6150 4650
Connection ~ 5650 3900
Wire Wire Line
	5650 3900 6650 3900
Wire Wire Line
	5650 3850 5650 3900
Wire Wire Line
	5650 3900 5650 3950
Wire Wire Line
	5650 5950 5650 6000
$Comp
L Device:Jumper_NC_Dual JP2
U 1 1 5BD2D7E0
P 7550 2450
F 0 "JP2" H 7550 2689 50  0000 C CNN
F 1 "A1" H 7550 2598 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x03_P2.54mm_Vertical" H 7550 2450 50  0001 C CNN
F 3 "" H 7550 2450 50  0001 C CNN
	1    7550 2450
	1    0    0    -1  
$EndComp
$Comp
L Device:Jumper_NC_Dual JP4
U 1 1 5BD2D7E6
P 6900 5350
F 0 "JP4" H 6900 5589 50  0000 C CNN
F 1 "A1" H 6900 5498 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x03_P2.54mm_Vertical" H 6900 5350 50  0001 C CNN
F 3 "" H 6900 5350 50  0001 C CNN
	1    6900 5350
	1    0    0    -1  
$EndComp
$Comp
L Device:Jumper_NC_Dual JP6
U 1 1 5BD2D7EC
P 8200 5350
F 0 "JP6" H 8200 5589 50  0000 C CNN
F 1 "A2" H 8200 5498 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x03_P2.54mm_Vertical" H 8200 5350 50  0001 C CNN
F 3 "" H 8200 5350 50  0001 C CNN
	1    8200 5350
	1    0    0    -1  
$EndComp
Wire Wire Line
	6900 5450 6900 5550
Wire Wire Line
	6150 5550 6900 5550
Wire Wire Line
	7550 5450 7550 5650
Wire Wire Line
	6150 5650 7550 5650
Wire Wire Line
	8200 5750 8200 5450
Wire Wire Line
	6150 5750 8200 5750
Wire Wire Line
	6650 5350 6650 4950
Wire Wire Line
	6650 4950 7300 4950
Wire Wire Line
	7300 4950 7300 5350
Wire Wire Line
	7300 4950 7950 4950
Wire Wire Line
	7950 4950 7950 5350
Connection ~ 7300 4950
Wire Wire Line
	6650 4950 6650 3900
Connection ~ 6650 4950
Connection ~ 6650 3900
Wire Wire Line
	6650 3900 6850 3900
Wire Wire Line
	7150 5350 7150 5500
Wire Wire Line
	7150 5500 7800 5500
Wire Wire Line
	7800 5500 7800 5350
Wire Wire Line
	7800 5500 8450 5500
Wire Wire Line
	8450 5500 8450 5350
Connection ~ 7800 5500
Wire Wire Line
	7150 5500 7150 6000
Wire Wire Line
	7150 6000 5650 6000
Connection ~ 7150 5500
Connection ~ 5650 6000
Wire Wire Line
	5650 6000 5650 6100
$Comp
L 74xx:74LS32 U1
U 1 1 5BD305B1
P 3600 5750
F 0 "U1" H 3600 6075 50  0000 C CNN
F 1 "74HCT32" H 3600 5984 50  0000 C CNN
F 2 "Package_DIP:DIP-14_W7.62mm_Socket" H 3600 5750 50  0001 C CNN
F 3 "" H 3600 5750 50  0001 C CNN
	1    3600 5750
	1    0    0    -1  
$EndComp
$Comp
L 74xx:74LS32 U1
U 2 1 5BD305EF
P 3600 6300
F 0 "U1" H 3600 6625 50  0000 C CNN
F 1 "74HCT32" H 3600 6534 50  0000 C CNN
F 2 "Package_DIP:DIP-14_W7.62mm_Socket" H 3600 6300 50  0001 C CNN
F 3 "" H 3600 6300 50  0001 C CNN
	2    3600 6300
	1    0    0    -1  
$EndComp
$Comp
L 74xx:74LS32 U1
U 3 1 5BD30629
P 3600 6850
F 0 "U1" H 3600 7175 50  0000 C CNN
F 1 "74HCT32" H 3600 7084 50  0000 C CNN
F 2 "Package_DIP:DIP-14_W7.62mm_Socket" H 3600 6850 50  0001 C CNN
F 3 "" H 3600 6850 50  0001 C CNN
	3    3600 6850
	1    0    0    -1  
$EndComp
$Comp
L 74xx:74LS32 U1
U 4 1 5BD30663
P 3600 7400
F 0 "U1" H 3600 7725 50  0000 C CNN
F 1 "74HCT32" H 3600 7634 50  0000 C CNN
F 2 "Package_DIP:DIP-14_W7.62mm_Socket" H 3600 7400 50  0001 C CNN
F 3 "" H 3600 7400 50  0001 C CNN
	4    3600 7400
	1    0    0    -1  
$EndComp
$Comp
L 74xx:74LS32 U1
U 5 1 5BD32FF5
P 4850 6850
F 0 "U1" H 5080 6896 50  0000 L CNN
F 1 "74HCT32" H 5080 6805 50  0000 L CNN
F 2 "Package_DIP:DIP-14_W7.62mm_Socket" H 4850 6850 50  0001 C CNN
F 3 "" H 4850 6850 50  0001 C CNN
	5    4850 6850
	1    0    0    -1  
$EndComp
$Comp
L power:VCC #PWR0107
U 1 1 5BD37515
P 4850 6300
F 0 "#PWR0107" H 4850 6150 50  0001 C CNN
F 1 "VCC" H 4850 6450 50  0000 C CNN
F 2 "" H 4850 6300 50  0001 C CNN
F 3 "" H 4850 6300 50  0001 C CNN
	1    4850 6300
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0108
U 1 1 5BD37542
P 4850 7400
F 0 "#PWR0108" H 4850 7150 50  0001 C CNN
F 1 "GND" H 4850 7250 50  0000 C CNN
F 2 "" H 4850 7400 50  0001 C CNN
F 3 "" H 4850 7400 50  0001 C CNN
	1    4850 7400
	1    0    0    -1  
$EndComp
Wire Wire Line
	4850 7400 4850 7350
Wire Wire Line
	4850 6350 4850 6300
$Comp
L Device:C C1
U 1 1 5BD3910A
P 5550 6900
F 0 "C1" H 5575 7000 50  0000 L CNN
F 1 ".1uf" H 5575 6800 50  0000 L CNN
F 2 "Capacitor_THT:C_Disc_D4.7mm_W2.5mm_P5.00mm" H 5588 6750 50  0001 C CNN
F 3 "" H 5550 6900 50  0001 C CNN
	1    5550 6900
	1    0    0    -1  
$EndComp
Wire Wire Line
	4850 6350 5550 6350
Wire Wire Line
	5550 6350 5550 6750
Connection ~ 4850 6350
Wire Wire Line
	4850 7350 5550 7350
Wire Wire Line
	5550 7350 5550 7050
Connection ~ 4850 7350
$Comp
L power:GND #PWR0109
U 1 1 5BD5E3DD
P 10650 2150
F 0 "#PWR0109" H 10650 1900 50  0001 C CNN
F 1 "GND" H 10650 2000 50  0000 C CNN
F 2 "" H 10650 2150 50  0001 C CNN
F 3 "" H 10650 2150 50  0001 C CNN
	1    10650 2150
	1    0    0    -1  
$EndComp
$Comp
L power:VCC #PWR0110
U 1 1 5BD5F6BA
P 10050 1650
F 0 "#PWR0110" H 10050 1500 50  0001 C CNN
F 1 "VCC" H 10050 1800 50  0000 C CNN
F 2 "" H 10050 1650 50  0001 C CNN
F 3 "" H 10050 1650 50  0001 C CNN
	1    10050 1650
	1    0    0    -1  
$EndComp
$Comp
L power:VCC #PWR0117
U 1 1 5BD6FE4B
P 8950 1600
F 0 "#PWR0117" H 8950 1450 50  0001 C CNN
F 1 "VCC" H 8950 1750 50  0000 C CNN
F 2 "" H 8950 1600 50  0001 C CNN
F 3 "" H 8950 1600 50  0001 C CNN
	1    8950 1600
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0118
U 1 1 5BD7142C
P 8850 1700
F 0 "#PWR0118" H 8850 1450 50  0001 C CNN
F 1 "GND" H 8850 1550 50  0000 C CNN
F 2 "" H 8850 1700 50  0001 C CNN
F 3 "" H 8850 1700 50  0001 C CNN
	1    8850 1700
	-1   0    0    -1  
$EndComp
Wire Wire Line
	10600 2150 10650 2150
Wire Wire Line
	10050 1650 10100 1650
$Comp
L Connector_Generic:Conn_02x06_Odd_Even J10
U 1 1 5BDB4E7C
P 10300 1850
F 0 "J10" H 10350 2267 50  0000 C CNN
F 1 "GPIO_A" H 10350 2176 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_2x06_P2.54mm_Horizontal" H 10300 1850 50  0001 C CNN
F 3 "" H 10300 1850 50  0001 C CNN
	1    10300 1850
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0125
U 1 1 5BDEE53E
P 11050 1650
F 0 "#PWR0125" H 11050 1400 50  0001 C CNN
F 1 "GND" H 11050 1500 50  0000 C CNN
F 2 "" H 11050 1650 50  0001 C CNN
F 3 "" H 11050 1650 50  0001 C CNN
	1    11050 1650
	1    0    0    -1  
$EndComp
Wire Wire Line
	10600 1650 11050 1650
$Comp
L power:VCC #PWR0132
U 1 1 5BDF751D
P 9700 2150
F 0 "#PWR0132" H 9700 2000 50  0001 C CNN
F 1 "VCC" H 9700 2300 50  0000 C CNN
F 2 "" H 9700 2150 50  0001 C CNN
F 3 "" H 9700 2150 50  0001 C CNN
	1    9700 2150
	-1   0    0    -1  
$EndComp
Wire Wire Line
	9700 2150 10100 2150
Text Label 6400 5150 2    60   ~ 0
SCK
Text Label 6450 5050 2    60   ~ 0
~IOXCS~
Wire Wire Line
	6150 5050 6450 5050
Wire Wire Line
	6150 5150 6400 5150
Wire Wire Line
	6150 5250 6400 5250
Wire Wire Line
	6150 5350 6400 5350
Text Label 6400 2250 2    60   ~ 0
SCK
Text Label 6450 2150 2    60   ~ 0
~IOXCS~
Wire Wire Line
	6150 2150 6450 2150
Wire Wire Line
	6150 2250 6400 2250
Wire Wire Line
	6150 2350 6400 2350
Wire Wire Line
	6150 2450 6400 2450
Wire Wire Line
	4900 1250 5150 1250
Wire Wire Line
	4900 1350 5150 1350
Wire Wire Line
	4900 1450 5150 1450
Wire Wire Line
	4900 1550 5150 1550
Wire Wire Line
	4900 1650 5150 1650
Wire Wire Line
	4900 1750 5150 1750
Wire Wire Line
	4900 1850 5150 1850
Wire Wire Line
	4900 1950 5150 1950
Wire Wire Line
	4900 2150 5150 2150
Wire Wire Line
	4900 2250 5150 2250
Wire Wire Line
	4900 2350 5150 2350
Wire Wire Line
	4900 2450 5150 2450
Wire Wire Line
	4900 2550 5150 2550
Wire Wire Line
	4900 2650 5150 2650
Wire Wire Line
	4900 2750 5150 2750
Wire Wire Line
	4900 2850 5150 2850
Text Label 4900 1250 0    60   ~ 0
GPB0
Text Label 4900 1350 0    60   ~ 0
GPB1
Text Label 4900 1450 0    60   ~ 0
GPB2
Text Label 4900 1550 0    60   ~ 0
GPB3
Text Label 4900 1650 0    60   ~ 0
GPB4
Text Label 4900 1750 0    60   ~ 0
GPB5
Text Label 4900 1850 0    60   ~ 0
GPB6
Text Label 4900 1950 0    60   ~ 0
GPB7
Text Label 4900 2150 0    60   ~ 0
GPA0
Text Label 4900 2250 0    60   ~ 0
GPA1
Text Label 4900 2350 0    60   ~ 0
GPA2
Text Label 4900 2450 0    60   ~ 0
GPA3
Text Label 4900 2550 0    60   ~ 0
GPA4
Text Label 4900 2650 0    60   ~ 0
GPA5
Text Label 4900 2750 0    60   ~ 0
GPA6
Text Label 4900 2850 0    60   ~ 0
GPA7
Text Label 4900 4150 0    60   ~ 0
GPD0
Text Label 4900 4250 0    60   ~ 0
GPD1
Text Label 4900 4350 0    60   ~ 0
GPD2
Text Label 4900 4450 0    60   ~ 0
GPD3
Text Label 4900 4550 0    60   ~ 0
GPD4
Text Label 4900 4650 0    60   ~ 0
GPD5
Text Label 4900 4750 0    60   ~ 0
GPD6
Text Label 4900 4850 0    60   ~ 0
GPD7
Wire Wire Line
	5150 4150 4900 4150
Wire Wire Line
	5150 4250 4900 4250
Wire Wire Line
	5150 4350 4900 4350
Wire Wire Line
	5150 4450 4900 4450
Wire Wire Line
	5150 4550 4900 4550
Wire Wire Line
	5150 4650 4900 4650
Wire Wire Line
	5150 4750 4900 4750
Wire Wire Line
	5150 4850 4900 4850
Text Label 4900 5050 0    60   ~ 0
GPC0
Text Label 4900 5150 0    60   ~ 0
GPC1
Text Label 4900 5250 0    60   ~ 0
GPC2
Text Label 4900 5350 0    60   ~ 0
GPC3
Text Label 4900 5450 0    60   ~ 0
GPC4
Text Label 4900 5550 0    60   ~ 0
GPC5
Text Label 4900 5650 0    60   ~ 0
GPC6
Text Label 4900 5750 0    60   ~ 0
GPC7
Wire Wire Line
	5150 5050 4900 5050
Wire Wire Line
	5150 5150 4900 5150
Wire Wire Line
	5150 5250 4900 5250
Wire Wire Line
	5150 5350 4900 5350
Wire Wire Line
	5150 5450 4900 5450
Wire Wire Line
	5150 5550 4900 5550
Wire Wire Line
	5150 5650 4900 5650
Wire Wire Line
	5150 5750 4900 5750
Text Label 9850 1750 0    60   ~ 0
GPA0
Text Label 9850 1850 0    60   ~ 0
GPA2
Text Label 9850 1950 0    60   ~ 0
GPA4
Text Label 9850 2050 0    60   ~ 0
GPA6
Text Label 10850 1750 2    60   ~ 0
GPA1
Text Label 10850 1850 2    60   ~ 0
GPA3
Text Label 10850 1950 2    60   ~ 0
GPA5
Text Label 10850 2050 2    60   ~ 0
GPA7
Wire Wire Line
	10100 1750 9850 1750
Wire Wire Line
	10100 1850 9850 1850
Wire Wire Line
	10100 1950 9850 1950
Wire Wire Line
	10100 2050 9850 2050
Wire Wire Line
	10600 1750 10850 1750
Wire Wire Line
	10600 1850 10850 1850
Wire Wire Line
	10600 1950 10850 1950
Wire Wire Line
	10600 2050 10850 2050
$Comp
L power:GND #PWR0111
U 1 1 5BED30D7
P 10650 3400
F 0 "#PWR0111" H 10650 3150 50  0001 C CNN
F 1 "GND" H 10650 3250 50  0000 C CNN
F 2 "" H 10650 3400 50  0001 C CNN
F 3 "" H 10650 3400 50  0001 C CNN
	1    10650 3400
	1    0    0    -1  
$EndComp
$Comp
L power:VCC #PWR0112
U 1 1 5BED30DD
P 10050 2900
F 0 "#PWR0112" H 10050 2750 50  0001 C CNN
F 1 "VCC" H 10050 3050 50  0000 C CNN
F 2 "" H 10050 2900 50  0001 C CNN
F 3 "" H 10050 2900 50  0001 C CNN
	1    10050 2900
	1    0    0    -1  
$EndComp
Wire Wire Line
	10600 3400 10650 3400
Wire Wire Line
	10050 2900 10100 2900
$Comp
L Connector_Generic:Conn_02x06_Odd_Even J7
U 1 1 5BED30E5
P 10300 3100
F 0 "J7" H 10350 3517 50  0000 C CNN
F 1 "GPIO_B" H 10350 3426 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_2x06_P2.54mm_Horizontal" H 10300 3100 50  0001 C CNN
F 3 "" H 10300 3100 50  0001 C CNN
	1    10300 3100
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0113
U 1 1 5BED30EB
P 11050 2900
F 0 "#PWR0113" H 11050 2650 50  0001 C CNN
F 1 "GND" H 11050 2750 50  0000 C CNN
F 2 "" H 11050 2900 50  0001 C CNN
F 3 "" H 11050 2900 50  0001 C CNN
	1    11050 2900
	1    0    0    -1  
$EndComp
Wire Wire Line
	10600 2900 11050 2900
$Comp
L power:VCC #PWR0114
U 1 1 5BED30F2
P 9700 3400
F 0 "#PWR0114" H 9700 3250 50  0001 C CNN
F 1 "VCC" H 9700 3550 50  0000 C CNN
F 2 "" H 9700 3400 50  0001 C CNN
F 3 "" H 9700 3400 50  0001 C CNN
	1    9700 3400
	-1   0    0    -1  
$EndComp
Wire Wire Line
	9700 3400 10100 3400
Text Label 9850 3000 0    60   ~ 0
GPB0
Text Label 9850 3100 0    60   ~ 0
GPB2
Text Label 9850 3200 0    60   ~ 0
GPB4
Text Label 9850 3300 0    60   ~ 0
GPB6
Text Label 10850 3000 2    60   ~ 0
GPB1
Text Label 10850 3100 2    60   ~ 0
GPB3
Text Label 10850 3200 2    60   ~ 0
GPB5
Text Label 10850 3300 2    60   ~ 0
GPB7
Wire Wire Line
	10100 3000 9850 3000
Wire Wire Line
	10100 3100 9850 3100
Wire Wire Line
	10100 3200 9850 3200
Wire Wire Line
	10100 3300 9850 3300
Wire Wire Line
	10600 3000 10850 3000
Wire Wire Line
	10600 3100 10850 3100
Wire Wire Line
	10600 3200 10850 3200
Wire Wire Line
	10600 3300 10850 3300
$Comp
L power:GND #PWR0115
U 1 1 5BED8C97
P 10650 4700
F 0 "#PWR0115" H 10650 4450 50  0001 C CNN
F 1 "GND" H 10650 4550 50  0000 C CNN
F 2 "" H 10650 4700 50  0001 C CNN
F 3 "" H 10650 4700 50  0001 C CNN
	1    10650 4700
	1    0    0    -1  
$EndComp
$Comp
L power:VCC #PWR0116
U 1 1 5BED8C9D
P 10050 4200
F 0 "#PWR0116" H 10050 4050 50  0001 C CNN
F 1 "VCC" H 10050 4350 50  0000 C CNN
F 2 "" H 10050 4200 50  0001 C CNN
F 3 "" H 10050 4200 50  0001 C CNN
	1    10050 4200
	1    0    0    -1  
$EndComp
Wire Wire Line
	10600 4700 10650 4700
Wire Wire Line
	10050 4200 10100 4200
$Comp
L Connector_Generic:Conn_02x06_Odd_Even J8
U 1 1 5BED8CA5
P 10300 4400
F 0 "J8" H 10350 4817 50  0000 C CNN
F 1 "GPIO_C" H 10350 4726 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_2x06_P2.54mm_Horizontal" H 10300 4400 50  0001 C CNN
F 3 "" H 10300 4400 50  0001 C CNN
	1    10300 4400
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0126
U 1 1 5BED8CAB
P 11050 4200
F 0 "#PWR0126" H 11050 3950 50  0001 C CNN
F 1 "GND" H 11050 4050 50  0000 C CNN
F 2 "" H 11050 4200 50  0001 C CNN
F 3 "" H 11050 4200 50  0001 C CNN
	1    11050 4200
	1    0    0    -1  
$EndComp
Wire Wire Line
	10600 4200 11050 4200
$Comp
L power:VCC #PWR0127
U 1 1 5BED8CB2
P 9700 4700
F 0 "#PWR0127" H 9700 4550 50  0001 C CNN
F 1 "VCC" H 9700 4850 50  0000 C CNN
F 2 "" H 9700 4700 50  0001 C CNN
F 3 "" H 9700 4700 50  0001 C CNN
	1    9700 4700
	-1   0    0    -1  
$EndComp
Wire Wire Line
	9700 4700 10100 4700
Text Label 9850 4300 0    60   ~ 0
GPC0
Text Label 9850 4400 0    60   ~ 0
GPC2
Text Label 9850 4500 0    60   ~ 0
GPC4
Text Label 9850 4600 0    60   ~ 0
GPC6
Text Label 10850 4300 2    60   ~ 0
GPC1
Text Label 10850 4400 2    60   ~ 0
GPC3
Text Label 10850 4500 2    60   ~ 0
GPC5
Text Label 10850 4600 2    60   ~ 0
GPC7
Wire Wire Line
	10100 4300 9850 4300
Wire Wire Line
	10100 4400 9850 4400
Wire Wire Line
	10100 4500 9850 4500
Wire Wire Line
	10100 4600 9850 4600
Wire Wire Line
	10600 4300 10850 4300
Wire Wire Line
	10600 4400 10850 4400
Wire Wire Line
	10600 4500 10850 4500
Wire Wire Line
	10600 4600 10850 4600
$Comp
L power:GND #PWR0128
U 1 1 5BED8CC9
P 10650 5950
F 0 "#PWR0128" H 10650 5700 50  0001 C CNN
F 1 "GND" H 10650 5800 50  0000 C CNN
F 2 "" H 10650 5950 50  0001 C CNN
F 3 "" H 10650 5950 50  0001 C CNN
	1    10650 5950
	1    0    0    -1  
$EndComp
$Comp
L power:VCC #PWR0129
U 1 1 5BED8CCF
P 10050 5450
F 0 "#PWR0129" H 10050 5300 50  0001 C CNN
F 1 "VCC" H 10050 5600 50  0000 C CNN
F 2 "" H 10050 5450 50  0001 C CNN
F 3 "" H 10050 5450 50  0001 C CNN
	1    10050 5450
	1    0    0    -1  
$EndComp
Wire Wire Line
	10600 5950 10650 5950
Wire Wire Line
	10050 5450 10100 5450
$Comp
L Connector_Generic:Conn_02x06_Odd_Even J9
U 1 1 5BED8CD7
P 10300 5650
F 0 "J9" H 10350 6067 50  0000 C CNN
F 1 "GPIO_D" H 10350 5976 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_2x06_P2.54mm_Horizontal" H 10300 5650 50  0001 C CNN
F 3 "" H 10300 5650 50  0001 C CNN
	1    10300 5650
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0130
U 1 1 5BED8CDD
P 11050 5450
F 0 "#PWR0130" H 11050 5200 50  0001 C CNN
F 1 "GND" H 11050 5300 50  0000 C CNN
F 2 "" H 11050 5450 50  0001 C CNN
F 3 "" H 11050 5450 50  0001 C CNN
	1    11050 5450
	1    0    0    -1  
$EndComp
Wire Wire Line
	10600 5450 11050 5450
$Comp
L power:VCC #PWR0131
U 1 1 5BED8CE4
P 9700 5950
F 0 "#PWR0131" H 9700 5800 50  0001 C CNN
F 1 "VCC" H 9700 6100 50  0000 C CNN
F 2 "" H 9700 5950 50  0001 C CNN
F 3 "" H 9700 5950 50  0001 C CNN
	1    9700 5950
	-1   0    0    -1  
$EndComp
Wire Wire Line
	9700 5950 10100 5950
Text Label 9850 5550 0    60   ~ 0
GPD0
Text Label 9850 5650 0    60   ~ 0
GPD2
Text Label 9850 5750 0    60   ~ 0
GPD4
Text Label 9850 5850 0    60   ~ 0
GPD6
Text Label 10850 5550 2    60   ~ 0
GPD1
Text Label 10850 5650 2    60   ~ 0
GPD3
Text Label 10850 5750 2    60   ~ 0
GPD5
Text Label 10850 5850 2    60   ~ 0
GPD7
Wire Wire Line
	10100 5550 9850 5550
Wire Wire Line
	10100 5650 9850 5650
Wire Wire Line
	10100 5750 9850 5750
Wire Wire Line
	10100 5850 9850 5850
Wire Wire Line
	10600 5550 10850 5550
Wire Wire Line
	10600 5650 10850 5650
Wire Wire Line
	10600 5750 10850 5750
Wire Wire Line
	10600 5850 10850 5850
Text Label 4250 5750 2    60   ~ 0
~EXTCS1~
Text Label 4250 6300 2    60   ~ 0
~EXTCS2~
Text Label 4250 6850 2    60   ~ 0
~EXTCS3~
Text Label 4250 7400 2    60   ~ 0
~EXTCS4~
Wire Wire Line
	3900 5750 4250 5750
Wire Wire Line
	3900 6300 4250 6300
Wire Wire Line
	3900 6850 4250 6850
Wire Wire Line
	3900 7400 4250 7400
Text Label 3050 5650 0    60   ~ 0
GPB4
Text Label 3050 6200 0    60   ~ 0
GPB5
Text Label 3050 6750 0    60   ~ 0
GPB6
Text Label 3050 7300 0    60   ~ 0
GPB7
Wire Wire Line
	3050 5650 3300 5650
Wire Wire Line
	3050 6200 3300 6200
Wire Wire Line
	3050 6750 3300 6750
Wire Wire Line
	3050 7300 3300 7300
Text Label 2950 5850 0    60   ~ 0
~AUXCS2~
Text Label 2950 6400 0    60   ~ 0
~AUXCS2~
Text Label 2950 6950 0    60   ~ 0
~AUXCS2~
Text Label 2950 7500 0    60   ~ 0
~AUXCS2~
Wire Wire Line
	2950 5850 3300 5850
Wire Wire Line
	2950 6950 3300 6950
Wire Wire Line
	2950 7500 3300 7500
Wire Wire Line
	2950 6400 3300 6400
Text Label 1650 7250 0    60   ~ 0
~AUXCS1~
Text Label 1650 7350 0    60   ~ 0
~AUXCS2~
Wire Wire Line
	2000 7250 1650 7250
Wire Wire Line
	2000 7350 1650 7350
Text Label 1700 7150 0    60   ~ 0
~IOXCS~
Wire Wire Line
	1700 7150 2000 7150
Text Label 1500 7150 2    60   ~ 0
SCK
Text Label 1500 7250 2    60   ~ 0
MISO
Text Label 1500 7350 2    60   ~ 0
MOSI
Wire Wire Line
	1250 7150 1500 7150
Wire Wire Line
	1250 7250 1500 7250
Wire Wire Line
	1250 7350 1500 7350
Text Label 8950 1900 0    60   ~ 0
MISO
Text Label 8950 2000 0    60   ~ 0
MOSI
Text Label 8950 1800 0    60   ~ 0
SCK
Wire Wire Line
	9200 1800 8950 1800
Wire Wire Line
	9200 2000 8950 2000
Wire Wire Line
	9200 1900 8950 1900
Text Label 8850 2100 0    60   ~ 0
~EXTCS1~
Wire Wire Line
	9200 2100 8850 2100
NoConn ~ -1600 4350
Text Label 6400 2350 2    60   ~ 0
MOSI
Text Label 6400 2450 2    60   ~ 0
MISO
Text Label 6400 5250 2    60   ~ 0
MOSI
Text Label 6400 5350 2    60   ~ 0
MISO
$Comp
L Connector_Generic:Conn_01x06 J3
U 1 1 5BD7A7BF
P 9400 1900
F 0 "J3" H 9319 1375 50  0000 C CNN
F 1 "SPI1" H 9319 1466 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x06_P2.54mm_Horizontal" H 9400 1900 50  0001 C CNN
F 3 "~" H 9400 1900 50  0001 C CNN
	1    9400 1900
	1    0    0    1   
$EndComp
Wire Wire Line
	8850 1700 9200 1700
Wire Wire Line
	8950 1600 9200 1600
$Comp
L power:VCC #PWR0119
U 1 1 5BE35401
P 8950 2950
F 0 "#PWR0119" H 8950 2800 50  0001 C CNN
F 1 "VCC" H 8950 3100 50  0000 C CNN
F 2 "" H 8950 2950 50  0001 C CNN
F 3 "" H 8950 2950 50  0001 C CNN
	1    8950 2950
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0120
U 1 1 5BE35407
P 8850 3050
F 0 "#PWR0120" H 8850 2800 50  0001 C CNN
F 1 "GND" H 8850 2900 50  0000 C CNN
F 2 "" H 8850 3050 50  0001 C CNN
F 3 "" H 8850 3050 50  0001 C CNN
	1    8850 3050
	-1   0    0    -1  
$EndComp
Text Label 8950 3250 0    60   ~ 0
MISO
Text Label 8950 3350 0    60   ~ 0
MOSI
Text Label 8950 3150 0    60   ~ 0
SCK
Wire Wire Line
	9200 3150 8950 3150
Wire Wire Line
	9200 3350 8950 3350
Wire Wire Line
	9200 3250 8950 3250
Text Label 8850 3450 0    60   ~ 0
~EXTCS2~
Wire Wire Line
	9200 3450 8850 3450
$Comp
L Connector_Generic:Conn_01x06 J4
U 1 1 5BE35415
P 9400 3250
F 0 "J4" H 9319 2725 50  0000 C CNN
F 1 "SPI2" H 9319 2816 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x06_P2.54mm_Horizontal" H 9400 3250 50  0001 C CNN
F 3 "~" H 9400 3250 50  0001 C CNN
	1    9400 3250
	1    0    0    1   
$EndComp
Wire Wire Line
	8850 3050 9200 3050
Wire Wire Line
	8950 2950 9200 2950
$Comp
L power:VCC #PWR0121
U 1 1 5BE411B8
P 8950 4100
F 0 "#PWR0121" H 8950 3950 50  0001 C CNN
F 1 "VCC" H 8950 4250 50  0000 C CNN
F 2 "" H 8950 4100 50  0001 C CNN
F 3 "" H 8950 4100 50  0001 C CNN
	1    8950 4100
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0122
U 1 1 5BE411BE
P 8850 4200
F 0 "#PWR0122" H 8850 3950 50  0001 C CNN
F 1 "GND" H 8850 4050 50  0000 C CNN
F 2 "" H 8850 4200 50  0001 C CNN
F 3 "" H 8850 4200 50  0001 C CNN
	1    8850 4200
	-1   0    0    -1  
$EndComp
Text Label 8950 4400 0    60   ~ 0
MISO
Text Label 8950 4500 0    60   ~ 0
MOSI
Text Label 8950 4300 0    60   ~ 0
SCK
Wire Wire Line
	9200 4300 8950 4300
Wire Wire Line
	9200 4500 8950 4500
Wire Wire Line
	9200 4400 8950 4400
Text Label 8850 4600 0    60   ~ 0
~EXTCS3~
Wire Wire Line
	9200 4600 8850 4600
$Comp
L Connector_Generic:Conn_01x06 J5
U 1 1 5BE411CC
P 9400 4400
F 0 "J5" H 9319 3875 50  0000 C CNN
F 1 "SPI3" H 9319 3966 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x06_P2.54mm_Horizontal" H 9400 4400 50  0001 C CNN
F 3 "~" H 9400 4400 50  0001 C CNN
	1    9400 4400
	1    0    0    1   
$EndComp
Wire Wire Line
	8850 4200 9200 4200
Wire Wire Line
	8950 4100 9200 4100
$Comp
L power:VCC #PWR0123
U 1 1 5BE411D4
P 8950 5450
F 0 "#PWR0123" H 8950 5300 50  0001 C CNN
F 1 "VCC" H 8950 5600 50  0000 C CNN
F 2 "" H 8950 5450 50  0001 C CNN
F 3 "" H 8950 5450 50  0001 C CNN
	1    8950 5450
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0124
U 1 1 5BE411DA
P 8850 5550
F 0 "#PWR0124" H 8850 5300 50  0001 C CNN
F 1 "GND" H 8850 5400 50  0000 C CNN
F 2 "" H 8850 5550 50  0001 C CNN
F 3 "" H 8850 5550 50  0001 C CNN
	1    8850 5550
	-1   0    0    -1  
$EndComp
Text Label 8950 5750 0    60   ~ 0
MISO
Text Label 8950 5850 0    60   ~ 0
MOSI
Text Label 8950 5650 0    60   ~ 0
SCK
Wire Wire Line
	9200 5650 8950 5650
Wire Wire Line
	9200 5850 8950 5850
Wire Wire Line
	9200 5750 8950 5750
Text Label 8850 5950 0    60   ~ 0
~EXTCS4~
Wire Wire Line
	9200 5950 8850 5950
$Comp
L Connector_Generic:Conn_01x06 J6
U 1 1 5BE411E8
P 9400 5750
F 0 "J6" H 9319 5225 50  0000 C CNN
F 1 "SPI4" H 9319 5316 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x06_P2.54mm_Horizontal" H 9400 5750 50  0001 C CNN
F 3 "~" H 9400 5750 50  0001 C CNN
	1    9400 5750
	1    0    0    1   
$EndComp
Wire Wire Line
	8850 5550 9200 5550
Wire Wire Line
	8950 5450 9200 5450
Wire Wire Line
	1700 5150 2000 5150
Connection ~ 1700 5150
Wire Wire Line
	1700 5250 2000 5250
Connection ~ 1700 5250
$Comp
L z80ctrl-iox:DS1306+ U4
U 1 1 5BF0FD4B
P 2750 1850
F 0 "U4" H 2750 2715 50  0000 C CNN
F 1 "DS1306+" H 2750 2624 50  0000 C CNN
F 2 "Package_DIP:DIP-16_W7.62mm_Socket" H 2750 1850 50  0001 L BNN
F 3 "Maxim Integrated" H 2750 1850 50  0001 L BNN
F 4 "https://www.digikey.com/product-detail/en/maxim-integrated/DS1306/DS1306-ND/445065?utm_source=snapeda&utm_medium=aggregator&utm_campaign=symbol" H 2750 1850 50  0001 L BNN "Field4"
F 5 "DIP-16 Maxim Integrated" H 2750 1850 50  0001 L BNN "Field5"
F 6 "Real Time Clock Serial 96Byte 16-Pin PDIP N" H 2750 1850 50  0001 L BNN "Field6"
F 7 "DS1306-ND" H 2750 1850 50  0001 L BNN "Field7"
F 8 "DS1306" H 2750 1850 50  0001 L BNN "Field8"
	1    2750 1850
	1    0    0    -1  
$EndComp
$Comp
L Device:Battery_Cell BT1
U 1 1 5BF3A87C
P 950 2050
F 0 "BT1" H 1068 2146 50  0000 L CNN
F 1 "Battery_Cell" H 1068 2055 50  0000 L CNN
F 2 "Battery:BatteryHolder_Keystone_3001_1x12mm" V 950 2110 50  0001 C CNN
F 3 "~" V 950 2110 50  0001 C CNN
	1    950  2050
	1    0    0    -1  
$EndComp
Wire Wire Line
	2050 1650 950  1650
Wire Wire Line
	950  1650 950  1850
Wire Wire Line
	950  2150 950  2550
$Comp
L power:GND #PWR02
U 1 1 5BF80AD5
P 1650 2600
F 0 "#PWR02" H 1650 2350 50  0001 C CNN
F 1 "GND" H 1650 2450 50  0000 C CNN
F 2 "" H 1650 2600 50  0001 C CNN
F 3 "" H 1650 2600 50  0001 C CNN
	1    1650 2600
	-1   0    0    -1  
$EndComp
Wire Wire Line
	1650 2600 1650 2550
Text Label 1700 1950 0    60   ~ 0
~AUXCS1~
Wire Wire Line
	1700 1950 2050 1950
Text Label 1800 2250 0    60   ~ 0
SCK
Text Label 1800 2150 0    60   ~ 0
MISO
Text Label 1800 2050 0    60   ~ 0
MOSI
Wire Wire Line
	2050 2250 1800 2250
Wire Wire Line
	2050 2150 1800 2150
Wire Wire Line
	2050 2050 1800 2050
NoConn ~ 3450 1350
NoConn ~ 3450 1450
NoConn ~ 3450 1650
NoConn ~ 3450 1750
$Comp
L power:VCC #PWR01
U 1 1 5C059609
P 1600 1300
F 0 "#PWR01" H 1600 1150 50  0001 C CNN
F 1 "VCC" H 1600 1450 50  0000 C CNN
F 2 "" H 1600 1300 50  0001 C CNN
F 3 "" H 1600 1300 50  0001 C CNN
	1    1600 1300
	-1   0    0    -1  
$EndComp
Wire Wire Line
	1600 1300 1600 1350
Wire Wire Line
	1600 1350 1600 2250
Wire Wire Line
	1600 2350 2050 2350
Connection ~ 1600 1350
Wire Wire Line
	1600 1350 1850 1350
Wire Wire Line
	2050 1550 1850 1550
Wire Wire Line
	1850 1550 1850 1350
Connection ~ 1850 1350
Wire Wire Line
	1850 1350 2050 1350
$Comp
L Device:Crystal_Small Y1
U 1 1 5C0BC9C6
P 3750 2000
F 0 "Y1" V 3796 1912 50  0000 R CNN
F 1 "Crystal_Small" V 3705 1912 50  0000 R CNN
F 2 "Crystal:Crystal_DS26_D2.0mm_L6.0mm_Horizontal" H 3750 2000 50  0001 C CNN
F 3 "~" H 3750 2000 50  0001 C CNN
	1    3750 2000
	0    -1   -1   0   
$EndComp
Wire Wire Line
	3450 1950 3650 1950
Wire Wire Line
	3650 1950 3650 1900
Wire Wire Line
	3650 1900 3750 1900
Wire Wire Line
	3450 2050 3650 2050
Wire Wire Line
	3650 2050 3650 2100
Wire Wire Line
	3650 2100 3750 2100
Wire Wire Line
	2050 1450 1650 1450
Wire Wire Line
	1650 1450 1650 2550
Connection ~ 1650 2550
Wire Wire Line
	1650 2550 2050 2550
Wire Wire Line
	950  2550 1400 2550
$Comp
L Device:C C4
U 1 1 5C1043E9
P 1400 2400
F 0 "C4" H 1425 2500 50  0000 L CNN
F 1 ".1uf" H 1425 2300 50  0000 L CNN
F 2 "Capacitor_THT:C_Disc_D4.7mm_W2.5mm_P5.00mm" H 1438 2250 50  0001 C CNN
F 3 "" H 1400 2400 50  0001 C CNN
	1    1400 2400
	1    0    0    -1  
$EndComp
Connection ~ 1400 2550
Wire Wire Line
	1400 2550 1650 2550
Wire Wire Line
	1400 2250 1600 2250
Connection ~ 1600 2250
Wire Wire Line
	1600 2250 1600 2350
$Comp
L Connector_Generic:Conn_01x02 J11
U 1 1 5BF3089F
P 1400 6850
F 0 "J11" H 1320 6525 50  0000 C CNN
F 1 "Conn_01x02" H 1320 6616 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x02_P2.54mm_Vertical" H 1400 6850 50  0001 C CNN
F 3 "~" H 1400 6850 50  0001 C CNN
	1    1400 6850
	-1   0    0    1   
$EndComp
Wire Wire Line
	1600 6850 2000 6850
Text Label 1900 6750 2    50   ~ 0
~AVRRST~
Wire Wire Line
	1600 6750 1900 6750
Text Label 6450 1250 2    50   ~ 0
~AVRRST~
Wire Wire Line
	6150 1250 6450 1250
Text Label 6450 4150 2    50   ~ 0
~AVRRST~
Wire Wire Line
	6150 4150 6450 4150
$EndSCHEMATC
