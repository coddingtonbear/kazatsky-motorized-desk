EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
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
L Sensor_Current:A1369xUA-10 U1
U 1 1 604704E4
P 3800 2800
F 0 "U1" H 3570 2846 50  0000 R CNN
F 1 "A1369xUA-10" H 3570 2755 50  0000 R CNN
F 2 "Sensor_Current:Allegro_SIP-3" H 4150 2700 50  0001 L CIN
F 3 "http://www.allegromicro.com/~/media/Files/Datasheets/A1369-Datasheet.ashx?la=en" H 3800 2800 50  0001 C CNN
	1    3800 2800
	1    0    0    -1  
$EndComp
$Comp
L Device:R_Small R1
U 1 1 60470D30
P 4300 2650
F 0 "R1" H 4241 2604 50  0000 R CNN
F 1 "1k" H 4241 2695 50  0000 R CNN
F 2 "coddingtonbear:0805_Milling" H 4300 2650 50  0001 C CNN
F 3 "~" H 4300 2650 50  0001 C CNN
	1    4300 2650
	-1   0    0    1   
$EndComp
$Comp
L Connector_Generic:Conn_01x03 J1
U 1 1 60471470
P 4800 2800
F 0 "J1" H 4880 2842 50  0000 L CNN
F 1 "HALL" H 4880 2751 50  0000 L CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x03_P2.54mm_Vertical" H 4800 2800 50  0001 C CNN
F 3 "~" H 4800 2800 50  0001 C CNN
	1    4800 2800
	1    0    0    -1  
$EndComp
$Comp
L Device:C_Small C1
U 1 1 604729B4
P 4300 2950
F 0 "C1" H 4392 2996 50  0000 L CNN
F 1 "20p" H 4392 2905 50  0000 L CNN
F 2 "coddingtonbear:0805_Milling" H 4300 2950 50  0001 C CNN
F 3 "~" H 4300 2950 50  0001 C CNN
	1    4300 2950
	1    0    0    -1  
$EndComp
Wire Wire Line
	4600 2700 4600 2500
Wire Wire Line
	4600 2500 4300 2500
Wire Wire Line
	4300 2550 4300 2500
Connection ~ 4300 2500
Wire Wire Line
	4300 2500 3800 2500
Wire Wire Line
	4600 2800 4300 2800
Wire Wire Line
	4300 2850 4300 2800
Connection ~ 4300 2800
Wire Wire Line
	4300 2800 4200 2800
Wire Wire Line
	4300 2750 4300 2800
Wire Wire Line
	4600 2900 4600 3050
Wire Wire Line
	4600 3050 4300 3050
Wire Wire Line
	4300 3050 4300 3100
Wire Wire Line
	4300 3100 3800 3100
Connection ~ 4300 3050
$EndSCHEMATC
