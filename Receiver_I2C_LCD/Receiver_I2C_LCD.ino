/*
* @file Receiver_I2C_LCD.ino
* hydroMazing Copyright (C) 2015 Cory J. Potter - All Rights Reserved
* You may use, distribute and modify this code under the
* terms of the LICENSE.txt
* NOT INTENDED FOR COMMERCIAL USE!
* You should have received a copy of the LICENSE.txt with
* this file. If not, please write to: <bitsandbots@gmail.com>
*/

/*
Arduino Networked Communications
Requires LCD Shield with buttons on Arduino Uno Pin A0
*/
/************************************************************************/

#include <Wire.h>
#include <Time.h>
#include <SPI.h>
#include <LowPower/LowPower.h>
#include "nRF24L01.h"
#include "RF24.h"
#include <EEPROM.h>
#include <EEPROMAnything/EEPROMAnything.h>
#include <RCSwitch.h>
//#include <DS3232RTC\DS3232RTC.h>
#include "C:\Users\Cory\Documents\Arduino\CoreConduit\libraries\commons.h"
#include "C:\Users\Cory\Documents\Arduino\CoreConduit\libraries\NoClock.h"
#include "C:\Users\Cory\Documents\Arduino\CoreConduit\libraries\TheTimeMan.h"
#include "CoreSettings.h"
#include "C:\Users\Cory\Documents\Arduino\CoreConduit\libraries\CoreSystem.h"
#include "C:\Users\Cory\Documents\Arduino\CoreConduit\libraries\CoreConduit_nRF.h"
#include "C:\Users\Cory\Documents\Arduino\CoreConduit\libraries\CoreRCSwitch.h"
#include "C:\Users\Cory\Documents\Arduino\CoreConduit\libraries\TheDecider.h"
#include "C:\Users\Cory\Documents\Arduino\CoreConduit\libraries\Alerts.h"
#include "C:\Users\Cory\Documents\Arduino\CoreConduit\libraries\TheNotifier.h"
#include "C:\Users\Cory\Documents\Arduino\CoreConduit\libraries\NetRecorder.h"
#include "C:\Users\Cory\Documents\Arduino\CoreConduit\libraries\SwitchRecorder.h"
#include "C:\Users\Cory\Documents\Arduino\CoreConduit\libraries\TheRecorder.h"
#include "C:\Users\Cory\Documents\Arduino\CoreConduit\libraries\AlertRecorder.h"
// #include <OneWire/OneWire.h>
// #include <DallasTemperature/DallasTemperature.h>
// #include "C:\Users\Cory\Documents\Arduino\CoreConduit\libraries\DallasTemp.h"
// #include "C:\Users\Cory\Documents\Arduino\CoreConduit\libraries\CoreSensors.h"
#include <LiquidCrystal.h>
#include "C:\Users\Cory\Documents\Arduino\CoreConduit\libraries\CoreLCD.h"

void setup()
{
	// If you want to set the aref to something other than 5v
	// analogReference(EXTERNAL);

	/************************************************************************/
	/*
	- CONNECTIONS: nRF24L01 Modules See:
	http://arduino-info.wikispaces.com/Nrf24L01-2.4GHz-HowTo
	1 - GND
	2 - VCC 3.3V !!! NOT 5V
	3 - CE to Arduino pin 9
	4 - CSN to Arduino pin 10
	5 - SCK to Arduino pin 13
	6 - MOSI to Arduino pin 11
	7 - MISO to Arduino pin 12
	8 - UNUSED

	NOTE! Power Problems:
	Many users have had trouble getting the nRF24L01 modules to work.
	Many times the problem is that the 3.3V Power to the module does not have enough current capability,
	or current surges cause problems. Here are suggestions:
	- Connect a .3.3 uF to 10 uF (MicroFarad) capacitor directly on the module from +3.3V to Gnd (Watch + and - !)
	[Some users say 10 uF or more..]
	- A separate 3.3V power supply (Maybe this one?)
	- An Arduino compatible, which has an added 3.3V regulator (But maybe add a .1 uF capacitor on the radio module).
	** This is especially important if you are connecting the module with jumper wires.

	*/
	/************************************************************************/
	pinMode(PIN_A0, INPUT);

	/************************************************************************/
	/*  Interrupts		                                                    */
	/************************************************************************/
	// attachInterrupt(0, interrupted, CHANGE);
	// two interrupts are available: #0 on pin 2,
	// and interrupt				 #1 on pin 3.
	/************************************************************************/
	// Start the serial port to communicate to the PC
	isDebugMode(1);
	/************************************************************************/
	/* I2C Communications                                                   */
	/************************************************************************/
	Wire.onReceive( receiveData );
	Wire.begin(MY_ADDRESS);
	/************************************************************************/
	clockFlag = setClock(0);
	/************************************************************************/
	//if ( ( readNetData() == false ) || ( readSwitchData() == false ) || (readAlertDataObjects() == false ) ) { Timer_Save_Settings.triggered = true; }

/************************************************************************/
// initialize LCD
/************************************************************************/
lcd.begin(numCols,numRows);   // initialize the lcd for 16 chars 2 lines

lcd.setBacklight(HIGH);
lcd.noCursor();
//-------- Write characters on the display ------------------
// NOTE: Cursor Position: (CHAR, LINE) start at 0
lcd.print(F("One moment..."));
/************************************************************************/

randomSeed( analogRead( PIN_A0 ) );
}

/*********************************
*        STaTe MaCHiNe          *
*********************************/
void loop()
{
	// YIELD to other processes and delay for the amount of time we want between readings
	if ( ( millis() - LcdButtonsDelay ) > 750 ) { read_LCD_buttons(); } else { /* delay */ }


	// I2C Communications
// 	if ( haveData > 0 )
// 	{ 
// 		receiveData( haveData );
// 		lastRxTimestamp = millis();// 		Timer_rxData.triggered = true;// 	}
	
	if ( requestSync ) { txRequest( requestSync, SEND_TO_ADDRESS ); requestSync = 0; }


	/************************************************************************/
	/*  Timers:  USE CAUTION!!										 		*/
	/************************************************************************/
	if ( ( checkTimer( &Timer_rxData ) ) || ( Timer_rxData.triggered == true ) )
	{
		time_millis = millis();
		time_diff = ( time_millis - lastRxTimestamp );
		Timer_rxData.triggered = false;
		if ( ( time_diff < Timer_rxData.interval ) &&  ( time_diff > 0 ) )
 		{ 
 		if ( DEBUG ) { Serial.print(F(" diff: ")); Serial.println(time_diff); }
			 Timer_Log.triggered = true;
		}
	}

	/************************************************************************/
	/*  Timers:																*/
	/*  USE CAUTION!!  call checkTimer ONCE for each timer unless			*/
	/*  RESETING the timer's interval is desired             				*/
	/************************************************************************/

	//Timer Timer_Log 			= { TIMER_LOG, 300000UL, true, false, 0, NULL };
	if( ( checkTimer( &Timer_Log ) ) || ( Timer_Log.triggered == true ) )
	{
		if (DEBUG) { Serial.print(F("free RAM: ")); Serial.println(freeRam()); }
		if ( napCheck(4) == true )
		{
			// turn off backlight
			pinMode( PIN10_CS, OUTPUT );
			digitalWrite( PIN10_CS, LOW);
			// turn off display
			lcd.noDisplay();
		}
		if ( Timer_Log.triggered )
		{ 
		
			Timer_Log.triggered = false;
		}
	}

	// Timer Timer_Save_Settings
	if ( ( checkTimer( &Timer_Save_Settings ) ) || ( Timer_Save_Settings.triggered ) )
	{
		saveNetData();
		saveSwitchData();
		saveAlertDataObjects();
		Timer_Save_Settings.triggered = false;
// 		delay(1000);
// 		readNetData();
// 		readSwitchData();
// 		readAlertDataObjects();
	}


	// Timer Timer_Alerts
	if ( checkTimer( &Timer_Alerts ) )
	{
		// Check for alerts here and use the buzzer
		checkAlerts();
		if ( countAlerts() > 0 ) { sendAudibleAlert(); }
	}

	/************************************************************************/
	/*  LCD Timer				                                            */
	/************************************************************************/
	if ( checkTimer( &Timer_Lcd ) )
	{
		LcdDisplay( modeSelect );
	}

	if ( ( checkTimer( &Timer_Lcd_Cycle ) ) && ( Timer_Lcd_Cycle.triggered == true ) )
	{ if (modeSelect == 0) { Timer_Lcd_Cycle.triggered = LcdPrintApps( 0 );} }
	// else? other modes

	if ( ( checkTimer( &Timer_Lcd_Scroller ) ) && ( Timer_Lcd_Scroller.triggered == true ) )
	{
		currentLetter = LcdScroller( currentLetter, lcdScrollerMessage );
	}

}

void LcdDisplay ( uint8_t modeSelected )
{
	// if you called this function then
	// assume we want it's timer running
	Timer_Lcd.state = true;

	modeSelect = modeSelected;
	// Allow Notifier to send notifications
	alertCounter = countAlerts();
	if( alertCounter > 0 ) {
		modeSelect = 1;
		if ( DEBUG ) { Serial.print(F("ac:  ")); Serial.println( alertCounter ); }
		if (DEBUG) { Serial.print(F("ms:  ")); Serial.println(modeSelect); }
	}

	Timer_Lcd_Cycle.triggered = false;
	Timer_Lcd_Scroller.triggered = false;
		
	switch ( modeSelect )
	{
		case 0:
		LcdPrintApps( 0 );
		LcdPrintSensors( 1 );
		// update timer
		checkTimer( &Timer_Lcd_Cycle );
		Timer_Lcd_Cycle.triggered = true;
		break;
		case 1:
		// Alert Mode
		Timer_Lcd_Scroller.triggered = true;
		LcdPrintAlertState( alertCounter );
		break;
		case 2:
		// Calibration Mode
		LcdClearRow( 0 );
		LcdClearRow( 1 );
		selectOption = LcdPrintCalibOptions( selectOption );
		break;
		case 3:
		// Program Mode
		LcdClearRow( 0 );
		LcdClearRow( 1 );
		selectOption = LcdPrintProgramOptions( selectOption );
		break;
	}
}

void LcdReset ( void )
{
	// reset the nap counter so we'll stay awake
	napTimeCounter = 0;
	pinMode( PIN10_CS, OUTPUT );
	digitalWrite( PIN10_CS, HIGH);
	LcdClearRow( 1 );
	LcdClearRow( 0 );
	lcd.display();
}


/************************************************************************/
/*   LCD Shield Buttons use PIN_A0                                      */
/************************************************************************/
void read_LCD_buttons( void )
{               // read the buttons

	int adc_key_in  = 0;
	adc_key_in = analogRead(PIN_A0);       // read the value from the sensor

	// buttons read are centered at these values: 0, 144, 329, 504, 741
	// we add approx 50 to those values and check to see if we are close
	// We make this the 1st option for speed reasons since it will be the most likely result
	
	if (adc_key_in < 1000)
	{
		// stop the timers from continuing on
		Timer_Lcd.state = false;
		Timer_Lcd_Cycle.triggered = false;
		Timer_Lcd_Scroller.triggered = false;
		LcdReset();
		
		LcdButtonsDelay = millis();

		if ( DEBUG ) { Serial.println(adc_key_in); }
		
		// For V1.1 us this threshold
		/*
		if (adc_key_in < 50)   return btnRIGHT;
		if (adc_key_in < 250)  return btnUP;
		if (adc_key_in < 450)  return btnDOWN;
		if (adc_key_in < 650)  return btnLEFT;
		if (adc_key_in < 850)  return btnSELECT;
		*/
		// For V1.0 comment the other threshold and use the one below:
		if (adc_key_in < btnRIGHT)
		{
			switch (modeSelect)
			{
				case 0:
				selectMode++;
				selectMode = showSelectMode( selectMode );
				break;
				case 1:
				LcdPrintAlertState(0);
				break;
				case 2:
				selectOption++;
				selectOption = LcdPrintCalibOptions( selectOption );
				LcdDisplay( modeSelect );
				break;
				case 3:
				selectOption++;
				//selectOption = LcdPrintProgramOptions( selectOption );
				LcdDisplay( modeSelect );
				break;
			}
		}
		else if (adc_key_in < btnUP)
		{
			LcdTimeRefresh();
			delay(1000);
			LcdReset();
			selectMode = 0;
			selectOption = 0;
			modeSelect = 0;
			LcdDisplay(0);
		}
		else if (adc_key_in < btnDOWN)
		{
			LcdReset();
			selectMode = 0;
			selectOption = 0;
			modeSelect = 0;
			lcd.noDisplay();
			pinMode( PIN10_CS, OUTPUT );
			digitalWrite( PIN10_CS, LOW);
		}
		else if (adc_key_in < btnLEFT)
		{
			selectMode = 0;
			selectOption = 0;
			modeSelect = 0;
			LcdDisplay( modeSelect );
		}
		else if (adc_key_in < btnSELECT)
		{
					if (DEBUG) { Serial.print(F("mS: ")); Serial.println(modeSelect); }
			switch (modeSelect)
			{
				case 0:
					modeSelect = selectMode;
					LcdDisplay( modeSelect );
					break;
				case 1:
					if ( DEBUG ) { printAlerts(); }
					if ( clearOneAlert() == true ) { requestSync = 2; LcdClearRow( 1 ); lcd.print(F("  Alert Cleared!  ")); }
					delay(1000);
					LcdDisplay( 0 );
					break;
				case 2:
//					if ( DEBUG ) { printAppliance( &Appliance_FeedPump ); }
					break;
				case 3:
					switch (selectOption)
					{
						case 0:
						if (DEBUG) { Serial.print(F("sO: ")); Serial.println( selectOption ); }
						//selectOption++;
//						if ( ( lcdOffered ) ) { saveAllDataObjects(); selectOption = 0; }
						lcdOffered = LcdOfferConfirm( lcdOffered );
						break;
						case 1:
						if (DEBUG) { Serial.print("sO: "); Serial.println( selectOption ); }
						//selectOption++;
						if ( ( lcdOffered ) )
						{
							if ( applianceOverride(0, false) == true )
							{ 
								requestSync = 1; LcdClearRow( 0 ); lcd.print(F("Appliance")); LcdClearRow( 1 ); lcd.print(F(" Override!")); selectOption = 0;
							}
						}
						delay(1000);
						lcdOffered = LcdOfferConfirm( lcdOffered );
						break;
						case 2:
						if ( lcdOffered ) { /*wipeMemory();*/ selectOption = 0; }
						lcdOffered = LcdOfferConfirm( lcdOffered );
						break;
						case 3:
						if (DEBUG) { Serial.print(F("sO: ")); Serial.println( selectOption ); }
						break;
					}
					break;
			} // end switch (modeSelect)
		} // end else if (SELECT)
	} // end if (adc_key < 1000)
}
uint8_t getAlertType( void )
{
	uint8_t r = 0;
	// Start at the beginning struct
	Alert *alert = &Alert_System;

	for (; alert != NULL; alert = alert->next ) {
		// TheNotifier returns true if an alert has been triggered
		// but only the initial trigger
		if ( alert->triggered )
		{
			r = alert->type;
			break;
		}
	}
	
	// add 1 to account for base 0
	return r++;
}

void sendAudibleAlert( void )
{

	uint8_t howManyTimes = getAlertType();

	for ( ; howManyTimes > 0; howManyTimes-- )
	{
		// send audible
		waka( TONE_PIN );
	}

}

bool checkAlerts( void )
{
	bool r = false;
	
	// Start at the beginning struct
	Alert *alert = &Alert_System;

	for (; alert != NULL; alert = alert->next ) {
		// TheNotifier returns true if an alert has been triggered
		// but only the initial trigger
		r = TheNotifier( alert );
	}
	return r;
}

void beep (int speakerPin, float noteFrequency, long noteDuration)
{
	int x;
	// Convert the frequency to microseconds
	float microsecondsPerWave = 1000000/noteFrequency;
	// Calculate how many HIGH/LOW cycles there are per millisecond
	float millisecondsPerCycle = 1000/(microsecondsPerWave * 2);
	// Multiply noteDuration * number or cycles per millisecond
	float loopTime = noteDuration * millisecondsPerCycle;
	// Play the note for the calculated loopTime.
	for (x=0;x<loopTime;x++)
	{
		digitalWrite(speakerPin,HIGH);
		delayMicroseconds(microsecondsPerWave);
		digitalWrite(speakerPin,LOW);
		delayMicroseconds(microsecondsPerWave);
	}
}

void waka( uint8_t speakerPin ) {
	for (int i=1000; i<3000; i=i*1.05) {
		beep(speakerPin,i,10);
	}
	delay(100);
	for (int i=2000; i>1000; i=i*.95) {
		beep(speakerPin,i,10);
	}
}

