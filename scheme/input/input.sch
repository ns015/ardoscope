EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr User 4724 4724
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L Device:R R3
U 1 1 61AA5E04
P 1550 2400
F 0 "R3" H 1620 2446 50  0000 L CNN
F 1 "8,3К" H 1620 2355 50  0000 L CNN
F 2 "" V 1480 2400 50  0001 C CNN
F 3 "~" H 1550 2400 50  0001 C CNN
	1    1550 2400
	1    0    0    -1  
$EndComp
$Comp
L Device:R R2
U 1 1 61AA698C
P 1550 2000
F 0 "R2" H 1620 2046 50  0000 L CNN
F 1 "8К" H 1620 1955 50  0000 L CNN
F 2 "" V 1480 2000 50  0001 C CNN
F 3 "~" H 1550 2000 50  0001 C CNN
	1    1550 2000
	1    0    0    -1  
$EndComp
Wire Wire Line
	1100 2600 1550 2600
$Comp
L power:GNDREF #PWR?
U 1 1 61AA8D8D
P 1550 2650
F 0 "#PWR?" H 1550 2400 50  0001 C CNN
F 1 "GNDREF" H 1555 2477 50  0000 C CNN
F 2 "" H 1550 2650 50  0001 C CNN
F 3 "" H 1550 2650 50  0001 C CNN
	1    1550 2650
	1    0    0    -1  
$EndComp
Text Label 2150 2150 0    50   ~ 0
A0
Text Label 900  2150 0    50   ~ 0
Input
Wire Wire Line
	1550 2600 2050 2600
Wire Wire Line
	1550 2150 1550 2200
Connection ~ 1550 2200
Wire Wire Line
	1550 2200 2050 2200
Wire Wire Line
	1550 2250 1550 2200
$Comp
L Device:R R1
U 1 1 61ADAE60
P 1300 2200
F 0 "R1" V 1093 2200 50  0000 C CNN
F 1 "92,5К" V 1184 2200 50  0000 C CNN
F 2 "" V 1230 2200 50  0001 C CNN
F 3 "~" H 1300 2200 50  0001 C CNN
	1    1300 2200
	0    1    1    0   
$EndComp
Wire Wire Line
	1450 2200 1550 2200
Wire Wire Line
	2050 1850 1550 1850
Text Label 2150 1800 0    50   ~ 0
Uref
Wire Wire Line
	1100 2200 1150 2200
Connection ~ 1550 2600
Wire Wire Line
	1550 2550 1550 2600
Wire Wire Line
	1550 2650 1550 2600
$Comp
L Graphic:SYM_Arrow_Tiny #SYM?
U 1 1 61B60DCA
P 2100 1850
F 0 "#SYM?" H 2100 1910 50  0001 C CNN
F 1 "SYM_Arrow_Tiny" H 2110 1800 50  0001 C CNN
F 2 "" H 2100 1850 50  0001 C CNN
F 3 "~" H 2100 1850 50  0001 C CNN
	1    2100 1850
	1    0    0    -1  
$EndComp
$Comp
L Graphic:SYM_Arrow_Tiny #SYM?
U 1 1 61B61BCF
P 2100 2200
F 0 "#SYM?" H 2100 2260 50  0001 C CNN
F 1 "SYM_Arrow_Tiny" H 2110 2150 50  0001 C CNN
F 2 "" H 2100 2200 50  0001 C CNN
F 3 "~" H 2100 2200 50  0001 C CNN
	1    2100 2200
	1    0    0    -1  
$EndComp
$Comp
L Graphic:SYM_Arrow_Tiny #SYM?
U 1 1 61B61F6C
P 2100 2600
F 0 "#SYM?" H 2100 2660 50  0001 C CNN
F 1 "SYM_Arrow_Tiny" H 2110 2550 50  0001 C CNN
F 2 "" H 2100 2600 50  0001 C CNN
F 3 "~" H 2100 2600 50  0001 C CNN
	1    2100 2600
	1    0    0    -1  
$EndComp
$Comp
L Graphic:SYM_Arrow_Tiny #SYM?
U 1 1 61B622B0
P 1050 2200
F 0 "#SYM?" H 1050 2260 50  0001 C CNN
F 1 "SYM_Arrow_Tiny" H 1060 2150 50  0001 C CNN
F 2 "" H 1050 2200 50  0001 C CNN
F 3 "~" H 1050 2200 50  0001 C CNN
	1    1050 2200
	-1   0    0    1   
$EndComp
$Comp
L Graphic:SYM_Arrow_Tiny #SYM?
U 1 1 61B626BE
P 1050 2600
F 0 "#SYM?" H 1050 2660 50  0001 C CNN
F 1 "SYM_Arrow_Tiny" H 1060 2550 50  0001 C CNN
F 2 "" H 1050 2600 50  0001 C CNN
F 3 "~" H 1050 2600 50  0001 C CNN
	1    1050 2600
	-1   0    0    1   
$EndComp
$Comp
L Reference_Voltage:TL431LP U1
U 1 1 61B85FFA
P 2800 2400
F 0 "U1" V 2846 2330 50  0000 R CNN
F 1 "TL431ACZ" V 2755 2330 50  0000 R CNN
F 2 "Package_TO_SOT_THT:TO-92_Inline" H 2800 2250 50  0001 C CIN
F 3 "http://www.ti.com/lit/ds/symlink/tl431.pdf" H 2800 2400 50  0001 C CIN
	1    2800 2400
	0    -1   -1   0   
$EndComp
$Comp
L power:GNDREF #PWR?
U 1 1 61B8E3F1
P 2800 2600
F 0 "#PWR?" H 2800 2350 50  0001 C CNN
F 1 "GNDREF" H 2805 2427 50  0000 C CNN
F 2 "" H 2800 2600 50  0001 C CNN
F 3 "" H 2800 2600 50  0001 C CNN
	1    2800 2600
	1    0    0    -1  
$EndComp
Wire Wire Line
	2800 2600 2800 2500
Wire Wire Line
	2800 2300 2800 2250
Wire Wire Line
	2800 2250 2650 2250
Wire Wire Line
	2650 2250 2650 2400
Wire Wire Line
	2650 2400 2700 2400
$Comp
L Device:R R4
U 1 1 61B8EEC5
P 2800 2050
F 0 "R4" H 2870 2096 50  0000 L CNN
F 1 "100" H 2870 2005 50  0000 L CNN
F 2 "" V 2730 2050 50  0001 C CNN
F 3 "~" H 2800 2050 50  0001 C CNN
	1    2800 2050
	1    0    0    -1  
$EndComp
Wire Wire Line
	2800 2250 2800 2200
Connection ~ 2800 2250
$Comp
L Graphic:SYM_Arrow_Tiny #SYM?
U 1 1 61B95416
P 3250 2250
F 0 "#SYM?" H 3250 2310 50  0001 C CNN
F 1 "SYM_Arrow_Tiny" H 3260 2200 50  0001 C CNN
F 2 "" H 3250 2250 50  0001 C CNN
F 3 "~" H 3250 2250 50  0001 C CNN
	1    3250 2250
	1    0    0    -1  
$EndComp
Wire Wire Line
	3200 2250 2800 2250
Wire Wire Line
	2800 1850 2800 1900
$Comp
L Graphic:SYM_Arrow_Tiny #SYM?
U 1 1 61B97385
P 2800 1800
F 0 "#SYM?" H 2800 1860 50  0001 C CNN
F 1 "SYM_Arrow_Tiny" H 2810 1750 50  0001 C CNN
F 2 "" H 2800 1800 50  0001 C CNN
F 3 "~" H 2800 1800 50  0001 C CNN
	1    2800 1800
	0    -1   -1   0   
$EndComp
Text Label 2850 1750 0    50   ~ 0
+5V
Text Label 3200 2200 0    50   ~ 0
Uref
$EndSCHEMATC
