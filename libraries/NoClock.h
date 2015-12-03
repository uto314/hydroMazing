/*
* @file NoClock.h
* Copyright (C) 2014,2015 Cory J. Potter - All Rights Reserved
* You may use, distribute and modify this code under the
* terms of the LICENSE.txt
* NOT INTENDED FOR COMMERCIAL USE!
* You should have received a copy of the LICENSE.txt with
* this file. If not, please write to: <bitsandbots@gmail.com>
*/

#ifndef __NOCLOCK_H__
#define __NOCLOCK_H__

#ifdef ARDUINO

//#define DS3232_I2C_ADDRESS 0x68
//const unsigned long DEFAULT_TIME = 1420092000; // 06:00:00, 1/1/2015

// Weekly Time Schedule?
const unsigned long oneDay =  86400UL;
const unsigned long oneHour = 3600UL;
// oneWeek = 604800 seconds * 1000 = milliseconds
// 4 weeks = 2419200 seconds
// 30 days = 2592000 seconds
// 365 days = 31536000 seconds
// unsigned long time_now = 0;
// unsigned long time_millis = 0;
// unsigned long time_diff = 0;
// bool clockFlag = false;

/************************************************************************/
/*
EEPROM memory AT24C32 with 32Kb
I2C real-time clock chip (RTC)
AT24C32 32K I2C EEPROM memory Includes a
LIR2032 rechargeable lithium battery
*/

const char *monthName[12] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

/************************************************************************/
/*  Real-Time Clock                                                     */
/************************************************************************/
void printDigits(int digits)
{
	// utility function for digital clock display: prints preceding colon and leading 0
	Serial.print(':');
	if(digits < 10)
	Serial.print('0');
	Serial.print(digits);
}

void digitalClockDisplay(void)
{
	// digital clock display of the time
	if (DEBUG) {
		Serial.print(hour());
		printDigits(minute());
		printDigits(second());
		Serial.print(' ');
		Serial.print(day());
		Serial.print(' ');
		Serial.print(month());
		Serial.print(' ');
		Serial.print(year());
		Serial.println();
	}
}

void print2digits(int number) {
	if (DEBUG)
	{
		if (number >= 0 && number < 10) {
			Serial.print('0');
		}
		Serial.print(number);
	}
}

void printRTC( void )
{
		if ( DEBUG ) { Serial.print(F("RTC: ")); digitalClockDisplay(); }
}

bool setClock( unsigned long _timestamp )
{
	bool r = false;
	time_now = now();
	if ( ( _timestamp > DEFAULT_TIME ) && ( time_now < _timestamp ) )
	{
		setTime( _timestamp );
		printRTC();
	}
	if ( time_now < DEFAULT_TIME )
	{
		setTime( DEFAULT_TIME );
		if ( DEBUG ) { Serial.print(F("NO ")); }
			printRTC();
	}
	return r;
}


#else
#error This code is only for use on Arduino.
#endif // ARDUINO

#endif // __NOCLOCK_H__