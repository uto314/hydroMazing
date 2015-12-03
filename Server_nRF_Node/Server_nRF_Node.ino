/*
* @file Server_nRF_Net.ino
* Copyright (C) 2015 Cory J. Potter - All Rights Reserved
* You may use, distribute and modify this code under the
* terms of the LICENSE.txt
* NOT INTENDED FOR COMMERCIAL USE!
* You should have received a copy of the LICENSE.txt with
* this file. If not, please write to: <bitsandbots@gmail.com>
*/

/*
Arduino Wireless Networked Communications
Manage Sensors and Appliances
Share Data with UI
*/

/************************************************************************/

#include <Wire.h>
#include <Time.h>
#include <LowPower\LowPower.h>
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include <EEPROM.h>
#include <EEPROMAnything/EEPROMAnything.h>
//#include <DHT.h>
#include <RCSwitch.h>
#include <DS3232RTC\DS3232RTC.h>
#include "C:\Users\Cory\Documents\Arduino\CoreConduit\libraries\commons.h"
#include "C:\Users\Cory\Documents\Arduino\CoreConduit\libraries\Clock.h"
#include "C:\Users\Cory\Documents\Arduino\CoreConduit\libraries\ClockSetter.h"
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
//#include "C:\Users\Cory\Documents\Arduino\CoreConduit\libraries\AppRecorder.h"
#include "C:\Users\Cory\Documents\Arduino\CoreConduit\libraries\AlertRecorder.h"
// #include "C:\Users\Cory\Documents\Arduino\CoreConduit\libraries\DallasTemp.h"
// #include "C:\Users\Cory\Documents\Arduino\CoreConduit\libraries\CoreSensors.h"

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

	// RF Rx Module for Remote Switches
	pinMode( RX_PIN, INPUT );
	// RF Tx Module for Remote Switches
	pinMode( TX_PIN, OUTPUT );
	// Piezo
	pinMode( TONE_PIN, OUTPUT );

/************************************************************************/
/*  Interrupts		                                                    */
/************************************************************************/
// attachInterrupt(0, interrupted, CHANGE);
// two interrupts are available: #0 on pin 2,
// and interrupt				 #1 on pin 3.

//rxSwitch.enableReceive(1);  // Receiver on interrupt 1 => that is pin #3
rxSwitch_Listening();
/************************************************************************/
	// Start the serial port to communicate to the PC
	isDebugMode(0);
	/************************************************************************/
	/* I2C Communications                                                   */
	/************************************************************************/
	Wire.onReceive( receiveData );
	Wire.begin(MY_ADDRESS);
	/************************************************************************/
	clockFlag = checkConsoleTime();
	if ( ( readNetData() == false ) || ( readSwitchData() == false ) ) { Timer_Save_Settings.triggered = true; }
	/************************************************************************/
	// Network Objects
	/************************************************************************/
	initializeRadio();
	radio.maskIRQ(1,1,1);
	/************************************************************************/
	randomSeed( analogRead( PIN_A0 ) );
	lastRxTimestamp = millis();
}

unsigned long rxCode = 0;

/*********************************
*        STaTe MaCHiNe          *
*********************************/
void loop()
{
	while ( rxSwitch.available() )
	{
		rxCode = rxSwitch_Check( rxCode );
		if ( rxCode > 0 )
		{
			// lookup rx code, if exists assume override command for AC outlet
			// returns switchNum in current memory, zero if no match.
			rcsBitLength = rxSwitch.getReceivedBitlength();
			uint8_t switchNum = lookupRCSwitch( rcsBitLength, rxCode );
			if ( switchNum > 0 )
			{
				if (DEBUG) { Serial.print(F("switch:  ")); Serial.println(switchNum); }
			}
		}
		rxCode = 0;
	}
	/************************************************************************/
	/* nRF24L01 Wireless Radio Communications                               */
	/************************************************************************/
	while ( radio.available() )
	{
		if ( receive_nRFdataObject( &rxDataObject ) )
		{
			/* We've got something */
			lastRxTimestamp = millis();
		}
	}

	// I2C Communications
	if ( haveData > 0 ) { receiveData( haveData ); }

	/************************************************************************/
	/*  Timers:  USE CAUTION!!										 		*/
	/************************************************************************/
	if ( checkTimer( &Timer_rxData ) )
	{
		time_millis = millis();
		time_diff = ( time_millis - lastRxTimestamp );
		if ( DEBUG ) { Serial.print(F(" diff: ")); Serial.println(time_diff); }
	}

	// Timer Timer_Log
	if ( checkTimer( &Timer_Log ) )
	{
		//TheDecider();
			Appliance * app = &Appliance_FeedPump;
			for (; app != NULL; app = app->next )
			{
			setDataObject( &rxDataObject, app );
			timeSinceLastAction( &rxDataObject );
			}
		// tx_nRF_SensorData( &Sensor_Voltage );
		// tx_nRF_ApplianceData( &Appliance_FeedPump );
		// I2C
		txSensorData( &Sensor_Voltage, SEND_TO_ADDRESS );
		delay(100);
		txApplianceData( &Appliance_FeedPump, SEND_TO_ADDRESS );
		// delay(100);
		// txAlertData( &Alert_System, SEND_TO_ADDRESS );
	}


	// Timer Timer_Save_Settings
	if ( ( checkTimer( &Timer_Save_Settings ) ) || ( Timer_Save_Settings.triggered ) )
	{
		saveNetData();
		saveSwitchData();
		saveAlertDataObjects();
		//saveApplianceDataObjects();
		Timer_Save_Settings.triggered = false;
// 		delay(1000);
// 		readNetData();// 		readSwitchData();// 		readAlertDataObjects();//		readApplianceDataObjects();

	}

	// Timer Timer_Sensor_Read
// 	if ( checkTimer( &Timer_Sensor_Read ) )
// 	{
// 		TheDecider();
// 	}

	// Timer Timer_Alerts
	if ( checkTimer( &Timer_Alerts ) )
	{
		// Check for alerts here and use the buzzer
		checkAlerts();
		if ( countAlerts() > 0 ) { sendAudibleAlert(); }
	}

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
//	uint8_t howManyTimes = 3;

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
