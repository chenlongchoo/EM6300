# MPLAB IDE generated this makefile for use with GNU make.
# Project: Three_Phase_Meter.mcp
# Date: Mon Mar 31 15:01:06 2014

AS = MPASMWIN.exe
CC = mcc18.exe
LD = mplink.exe
AR = mplib.exe
RM = rm

Three_Phase_Meter.cof : main.o mcu.o utility.o power.o adcon.o registers.o rtcc.o
	$(LD) /p18F46K20 /l"C:\MCC18\lib" "18f46k20.lkr" "main.o" "mcu.o" "utility.o" "power.o" "adcon.o" "registers.o" "rtcc.o" /u_CRUNTIME /z__MPLAB_BUILD=1 /m"Three_Phase_Meter.map" /w /o"Three_Phase_Meter.cof"

main.o : main.c ../../../../../MCC18/h/delays.h ../../../../../MCC18/h/timers.h ../../../../../MCC18/h/stdlib.h ../../../../../MCC18/h/math.h ../../../../../MCC18/h/stdio.h ../../../../../MCC18/h/string.h ../Microchip/Include/uart2.h ../../../../../MCC18/h/eep.h ../Microchip/Include/MDD\ File\ System/SD-SPI.h ../Microchip/Include/MDD\ File\ System/FSIO.h ../Common/adc.h main.h ../Common/mcu.h power.h ../Common/registers.h rtcc.h ../Common/utility.h main.c ../../../../../MCC18/h/p18cxxx.h ../../../../../MCC18/h/p18f46k20.h ../../../../../MCC18/h/pconfig.h HardwareProfile.h ../Microchip/Include/GenericTypeDefs.h ../../../../../MCC18/h/stddef.h ../Microchip/Include/Compiler.h ../../../../../MCC18/h/stdarg.h ../Common/AppConfig.h FSconfig.h ../Microchip/Include/MDD\ File\ System/FSDefs.h eeprom.h
	$(CC) -p=18F46K20 /i"..\Common" -I"..\Microchip\Include\PIC18 salloc" -I"..\Microchip\Include\MDD File System" -I"." -I"..\Microchip\Include" -I"C:\MCC18\h" "main.c" -fo="main.o" -DTHREE_PHASE_BOTTOM_BOARD -mL -Ou- -Ot- -Ob- -Op- -Or- -Od- -Opa-

mcu.o : ../common/mcu.c ../../../../../MCC18/h/delays.h ../../../../../MCC18/h/stdlib.h ../../../../../MCC18/h/string.h ../../../../../MCC18/h/usart.h ../../../../../MCC18/h/stdio.h ../Microchip/Include/uart2.h ../common/mcu.h ../common/menu.h ../common/registers.h ../common/utility.h main.h power.h ../../../../../MCC18/h/eep.h rtcc.h ../common/mcu.c ../../../../../MCC18/h/p18cxxx.h ../../../../../MCC18/h/p18f46k20.h ../../../../../MCC18/h/stddef.h ../../../../../MCC18/h/pconfig.h HardwareProfile.h ../Microchip/Include/GenericTypeDefs.h ../Microchip/Include/Compiler.h ../../../../../MCC18/h/stdarg.h ../common/AppConfig.h eeprom.h
	$(CC) -p=18F46K20 /i"..\Common" -I"..\Microchip\Include\PIC18 salloc" -I"..\Microchip\Include\MDD File System" -I"." -I"..\Microchip\Include" -I"C:\MCC18\h" "..\common\mcu.c" -fo="mcu.o" -DTHREE_PHASE_BOTTOM_BOARD -mL -Ou- -Ot- -Ob- -Op- -Or- -Od- -Opa-

utility.o : ../Common/utility.c ../../../../../MCC18/h/stdio.h ../../../../../MCC18/h/stdlib.h ../../../../../MCC18/h/math.h ../../../../../MCC18/h/string.h ../Microchip/Include/uart2.h ../Common/utility.h ../Common/registers.h main.h rtcc.h ../Common/utility.c ../../../../../MCC18/h/stdarg.h ../../../../../MCC18/h/stddef.h HardwareProfile.h ../Microchip/Include/GenericTypeDefs.h ../Microchip/Include/Compiler.h ../../../../../MCC18/h/p18cxxx.h ../../../../../MCC18/h/p18f46k20.h ../Common/AppConfig.h
	$(CC) -p=18F46K20 /i"..\Common" -I"..\Microchip\Include\PIC18 salloc" -I"..\Microchip\Include\MDD File System" -I"." -I"..\Microchip\Include" -I"C:\MCC18\h" "..\Common\utility.c" -fo="utility.o" -DTHREE_PHASE_BOTTOM_BOARD -mL -Ou- -Ot- -Ob- -Op- -Or- -Od- -Opa-

power.o : power.c ../../../../../MCC18/h/stdio.h ../../../../../MCC18/h/stdlib.h ../../../../../MCC18/h/string.h ../Microchip/Include/uart2.h main.h ../Common/mcu.h power.h ../Common/registers.h ../Common/utility.h rtcc.h ../../../../../MCC18/h/delays.h ../../../../../MCC18/h/spi.h power.c HardwareProfile.h ../Microchip/Include/GenericTypeDefs.h ../../../../../MCC18/h/stddef.h ../Microchip/Include/Compiler.h ../../../../../MCC18/h/p18cxxx.h ../../../../../MCC18/h/p18f46k20.h ../../../../../MCC18/h/stdarg.h ../Common/AppConfig.h ../../../../../MCC18/h/pconfig.h
	$(CC) -p=18F46K20 /i"..\Common" -I"..\Microchip\Include\PIC18 salloc" -I"..\Microchip\Include\MDD File System" -I"." -I"..\Microchip\Include" -I"C:\MCC18\h" "power.c" -fo="power.o" -DTHREE_PHASE_BOTTOM_BOARD -mL -Ou- -Ot- -Ob- -Op- -Or- -Od- -Opa-

adcon.o : ../Common/adcon.c ../Common/adc.h ../Common/adcon.h ../../../../../MCC18/h/pconfig.h ../../../../../MCC18/h/p18cxxx.h ../../../../../MCC18/h/p18f46k20.h ../Microchip/Include/GenericTypeDefs.h ../../../../../MCC18/h/stddef.h
	$(CC) -p=18F46K20 /i"..\Common" -I"..\Microchip\Include\PIC18 salloc" -I"..\Microchip\Include\MDD File System" -I"." -I"..\Microchip\Include" -I"C:\MCC18\h" "..\Common\adcon.c" -fo="adcon.o" -DTHREE_PHASE_BOTTOM_BOARD -mL -Ou- -Ot- -Ob- -Op- -Or- -Od- -Opa-

registers.o : ../Common/registers.c ../../../../../MCC18/h/stdio.h ../../../../../MCC18/h/stdlib.h ../../../../../MCC18/h/string.h ../Microchip/Include/uart2.h ../Common/registers.h ../Common/registers.c HardwareProfile.h ../Microchip/Include/GenericTypeDefs.h ../../../../../MCC18/h/stddef.h ../Microchip/Include/Compiler.h ../../../../../MCC18/h/p18cxxx.h ../../../../../MCC18/h/p18f46k20.h ../../../../../MCC18/h/stdarg.h
	$(CC) -p=18F46K20 /i"..\Common" -I"..\Microchip\Include\PIC18 salloc" -I"..\Microchip\Include\MDD File System" -I"." -I"..\Microchip\Include" -I"C:\MCC18\h" "..\Common\registers.c" -fo="registers.o" -DTHREE_PHASE_BOTTOM_BOARD -mL -Ou- -Ot- -Ob- -Op- -Or- -Od- -Opa-

rtcc.o : rtcc.c ../../../../../MCC18/h/delays.h ../../../../../MCC18/h/stdio.h ../../../../../MCC18/h/string.h ../../../../../MCC18/h/stdlib.h ../Microchip/Include/uart2.h rtcc.h ../Common/utility.h ../../../../../MCC18/h/eep.h rtcc.c ../../../../../MCC18/h/p18cxxx.h ../../../../../MCC18/h/p18f46k20.h ../../../../../MCC18/h/stdarg.h ../../../../../MCC18/h/stddef.h HardwareProfile.h ../Microchip/Include/GenericTypeDefs.h ../Microchip/Include/Compiler.h ../../../../../MCC18/h/pconfig.h
	$(CC) -p=18F46K20 /i"..\Common" -I"..\Microchip\Include\PIC18 salloc" -I"..\Microchip\Include\MDD File System" -I"." -I"..\Microchip\Include" -I"C:\MCC18\h" "rtcc.c" -fo="rtcc.o" -DTHREE_PHASE_BOTTOM_BOARD -mL -Ou- -Ot- -Ob- -Op- -Or- -Od- -Opa-

clean : 
	$(RM) "main.o" "mcu.o" "utility.o" "power.o" "adcon.o" "registers.o" "rtcc.o" "Three_Phase_Meter.cof" "Three_Phase_Meter.hex" "Three_Phase_Meter.map"

