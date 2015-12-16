/*
* @file Controller_nRF.ino
* hydroMazing Copyright (C) 2015 Cory J. Potter - All Rights Reserved
* You may use, distribute and modify this code under the
* terms of the LICENSE.txt
* NOT INTENDED FOR COMMERCIAL USE!
* You should have received a copy of the LICENSE.txt with
* this file. If not, please write to: <bitsandbots@gmail.com>
*/

/*
Arduino Networked Communications
Requires nRF module
*/


/************************************************************************/
/**

Arduino Uno

*I2C:
A4 (SDA)
A5 (SCL)

**/
/************************************************************************/

#include <Wire.h>
#include <Time.h>
#include <LowPower\LowPower.h>
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include <EEPROM.h>
#include <EEPROMAnything/EEPROMAnything.h>
#include <RCSwitch.h>
#include <DHT.h>
#include <OneWire/OneWire.h>
#include <DallasTemperature/DallasTemperature.h>
//#include <DS3232RTC\DS3232RTC.h>
#include "C:\Users\Cory\Documents\Arduino\CoreConduit\libraries\commons.h"
//#include "C:\Users\Cory\Documents\Arduino\CoreConduit\libraries\Clock.h"
#include "C:\Users\Cory\Documents\Arduino\CoreConduit\libraries\NoClock.h"
//#include "C:\Users\Cory\Documents\Arduino\CoreConduit\libraries\ClockSetter.h"
#include "C:\Users\Cory\Documents\Arduino\CoreConduit\libraries\TheTimeMan.h"
#include "CoreSettings.h"
#include "C:\Users\Cory\Documents\Arduino\CoreConduit\libraries\CoreSystem.h"
#include "C:\Users\Cory\Documents\Arduino\CoreConduit\libraries\CoreConduit_nRF.h"
#include "C:\Users\Cory\Documents\Arduino\CoreConduit\libraries\CoreRCSwitch.h"
#include "C:\Users\Cory\Documents\Arduino\CoreConduit\libraries\TheDecider.h"
//#include "C:\Users\Cory\Documents\Arduino\CoreConduit\libraries\Alerts.h"
//#include "C:\Users\Cory\Documents\Arduino\CoreConduit\libraries\TheNotifier.h"
#include "C:\Users\Cory\Documents\Arduino\CoreConduit\libraries\NetRecorder.h"
#include "C:\Users\Cory\Documents\Arduino\CoreConduit\libraries\SwitchRecorder.h"
#include "C:\Users\Cory\Documents\Arduino\CoreConduit\libraries\TheRecorder.h"
#include "C:\Users\Cory\Documents\Arduino\CoreConduit\libraries\AppRecorder.h"
//#include "C:\Users\Cory\Documents\Arduino\CoreConduit\libraries\AlertRecorder.h"
#include "C:\Users\Cory\Documents\Arduino\CoreConduit\libraries\DallasTemp.h"
#include "C:\Users\Cory\Documents\Arduino\CoreConduit\libraries\CoreSensors.h"

void setup()
{
	/************************************************************************/
	// RF Tx Module for Remote Switches
	// pinMode( TX_PIN, OUTPUT );

	//pinMode(Sensor_Flow.pin, INPUT);
	//digitalWrite(Sensor_Flow.pin, HIGH);

	/************************************************************************/
	/*  Interrupts		                                                    */
	/************************************************************************/
	// attachInterrupt(0, interrupted, CHANGE);
	// two interrupts are available: #0 on pin 2,
	// and interrupt				 #1 on pin 3.
	
	// The Hall-effect sensor is connected to pin 2 which uses interrupt 0.
	// Configured to trigger on a FALLING state change (transition from HIGH
	// state to LOW state)

	//attachInterrupt(0, pulseCounter, FALLING);

	/************************************************************************/
	// Start the serial port to communicate to the PC
	isDebugMode(0);
	/************************************************************************/
	/* I2C Communications                                                   */
	/************************************************************************/
	//Wire.onReceive ( receiveData );
	Wire.begin (MY_ADDRESS);
	/************************************************************************/
	clockFlag = setClock( 0 );
	//clockFlag = checkConsoleTime();
	// if ( ( readNetData() == false ) || ( readSwitchData() == false ) ) { Timer_Save_Settings.triggered = true; }
	/************************************************************************/
	// Address
	initializeRadio();
	/************************************************************************/

	/************************************************************************/
	randomSeed( analogRead( PIN_A0 ) );
	dht.begin();
	startupSensors();
	//setupPIR();
}

/*********************************
*        STaTe MaCHiNe          *
*********************************/
void loop()
{
	readSensors();
	/************************************************************************/
	/* nRF24L01 Wireless Radio Communications                               */
	/************************************************************************/
	while ( radio.available() )
	{
		if ( receive_nRFdataObject( &rxDataObject ) )
		{
			/* We've got something */
			lastRxTimestamp = millis();
			Timer_rxData.triggered = true;
		}
	}

	// I2C Communications
	//if ( haveData > 0 ) { receiveData( haveData ); }

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
		TheDecider();
		/*
		Appliance * app = &Appliance_FeedPump;
		for (; app != NULL; app = app->next )
		{
		setDataObject( &rxDataObject, app );
		timeSinceLastAction( &rxDataObject );
		}
		*/
		
		tx_nRF_SensorData( &Sensor_Voltage );
		tx_nRF_ApplianceData( &Appliance_FeedPump );
		// I2C
		// txSensorData( &Sensor_Voltage, SEND_TO_ADDRESS );
		// delay(100);
		// txApplianceData( &Appliance_FeedPump, SEND_TO_ADDRESS );
		// delay(100);
		// txAlertData( &Alert_System, SEND_TO_ADDRESS );
		Timer_Log.triggered = false;
	}

	// Timer Timer_Save_Settings
	if ( ( checkTimer( &Timer_Save_Settings ) ) || ( Timer_Save_Settings.triggered ) )
	{
		saveNetData();
		saveSwitchData();
		saveApplianceDataObjects();
		Timer_Save_Settings.triggered = false;
	}

}
