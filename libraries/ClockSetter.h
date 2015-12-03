/*
* @file ClockSetter.h
* Copyright (C) 2014 Cory J. Potter - All Rights Reserved
* You may use, distribute and modify this code under the
* terms of the LICENSE.txt
* NOT INTENDED FOR COMMERCIAL USE!
* You should have received a copy of the LICENSE.txt with
* this file. If not, please write to: <bitsandbots@gmail.com>
*/

#ifndef __CLOCKSETTER_H__
#define __CLOCKSETTER_H__

#ifdef ARDUINO

bool getTime(const char *str)
{
	int Hour, Min, Sec;

	if (sscanf(str, "%d:%d:%d", &Hour, &Min, &Sec) != 3) return false;
	tm.Hour = Hour;
	tm.Minute = Min;
	tm.Second = Sec;
	return true;
}

bool getDate(const char *str)
{
	char Month[12];
	int Day, Year;
	uint8_t monthIndex;

	if (sscanf(str, "%s %d %d", Month, &Day, &Year) != 3) return false;
	for (monthIndex = 0; monthIndex < 12; monthIndex++) {
		if (strcmp(Month, monthName[monthIndex]) == 0) break;
	}
	if (monthIndex >= 12) return false;
	tm.Day = Day;
	tm.Month = monthIndex + 1;
	tm.Year = CalendarYrToTm(Year);
	return true;
}

bool checkConsoleTime( void )
{
	bool r = false;

	// get the date and time the compiler was run
	// and configure the RTC with this info
	if ( ( getDate(__DATE__) && getTime(__TIME__) ) )
	{
		//setTime(tm.Hour, tm.Minute, tm.Second, tm.Day, tm.Month, tmYearToCalendar(tm.Year));
		time_now = makeTime(tm);
		r = setClock( time_now );
		if ( DEBUG ) { Serial.print(F("Console ")); Serial.print(__TIME__); Serial.print(F(", Date = " )); Serial.println(__DATE__); }
	}
	return r;
}

#else
#error This code is only for use on Arduino.
#endif // ARDUINO

#endif // __CLOCKSETTER_H__
