/*
* @file Clock.h
* Copyright (C) 2014,2015 Cory J. Potter - All Rights Reserved
* You may use, distribute and modify this code under the
* terms of the LICENSE.txt
* NOT INTENDED FOR COMMERCIAL USE!
* You should have received a copy of the LICENSE.txt with
* this file. If not, please write to: <bitsandbots@gmail.com>
*/

#ifndef __CLOCK_H__
#define __CLOCK_H__

#ifdef ARDUINO

//#define DS3232_I2C_ADDRESS 0x68
//const unsigned long DEFAULT_TIME = 1420092000; // 06:00:00, 1/1/2015
//static time_t tLast;
time_t t;
tmElements_t tm;

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

/**
bool set(time_t t)
{
bool r = false;
tmElements_t tm;
breakTime(t, tm);
tm.Second |= 0x80;  // stop the clock
r = write(tm);
tm.Second &= 0x7f;  // start the clock
r = write(tm);
return r;
}
**/

//Print an integer in "00" format (with leading zero),
//followed by a delimiter character to Serial.
//Input value assumed to be between 0 and 99.
void printI00(int val, char delim)
{
	if (val < 10) Serial.print('0');
	Serial.print(val, DEC);
	if (delim > 0) Serial.print(delim);
	return;
}

//print time to Serial
// void printTime(time_t t)
// {
// 	printI00(hour(t), ':');
// 	printI00(minute(t), ':');
// 	printI00(second(t), ' ');
// }

//print date to Serial
// void printDate(time_t t)
// {
// 	printI00(day(t), 0);
// 	Serial.print( monthShortStr(month(t)) );
// 	Serial.print(year(t), DEC );
// }


//print date and time to Serial
// void printDateTime(time_t t)
// {
// 	printDate(t);
// 	Serial.print(' ');
// 	printTime(t);
// }

void printRTC( void )
{
	if ( DEBUG ) { Serial.print(F("RTC: ")); digitalClockDisplay(); }
}

bool setClock( unsigned long _timestamp )
{
	bool r = false;
	time_now = now();
	if ( time_now < DEFAULT_TIME ) { setTime( DEFAULT_TIME ); }
	if ( ( _timestamp > DEFAULT_TIME ) && ( time_now < _timestamp ) )
	{
		setTime( _timestamp );
		time_now = now();
		RTC.set(time_now);        //use the time_t value to ensure correct weekday is set
		if (timeStatus() == timeSet) { r = true; }
	}
	else
	{
		// else time_now is > than _timestamp
		setSyncProvider(RTC.get);
		if (timeStatus() == timeSet) { r = true; }
	}
	if ( r ) { printRTC(); }
	return r;
}


#else
#error This code is only for use on Arduino.
#endif // ARDUINO

#endif // __CLOCK_H__