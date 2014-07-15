This firmware consist of few add in features

Hardware changes
- Super capacitor C70,C73 (MCU_VBAT) and C71,C72 (MAXQ_VBAT) are added
- VSENSE (PORTBbits.RB3)(input pins) to detect the power trip or power failures 
- VCONTROL (PORTDbits.RD2) to turn on or off the MAXQ_VBAT discharging circuit
- Super capacitor charging and discharging circuit were added


firmware understanding 
- timer3 was created to generate interrupt per 50 ms. 
	-This timing was used to triggering the flag_StartDoReadingFlag. 
		if( StartDoReadingFlag == 1 )
			{			
				ResetSPIDevices();
				DoReadings();
				ComputeTotalRealEnergy();
				StartDoReadingFlag = 0;
				#ifdef APP_USE_MCU
				{
					MCUTasks();
				}
				#endif
				
				Update_Max_Min();
			}
	-This timing was used to triggering the flag_SAVE_RTCC. 
		if (SAVE_TRCC)
			{	SaveRTCCToEEPROM();	
			}