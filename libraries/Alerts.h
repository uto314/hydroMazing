/*
* @file Alerts.h
* Copyright (C) 2014 Cory J. Potter - All Rights Reserved
* You may use, distribute and modify this code under the
* terms of the LICENSE.txt
* NOT INTENDED FOR COMMERCIAL USE!
* You should have received a copy of the LICENSE.txt with
* this file. If not, please write to: <bitsandbots@gmail.com>
*/

#ifndef __ALERTS_H__
#define __ALERTS_H__

#ifdef ARDUINO

enum ALERT_TYPE {

	ALERT_TEMP, /* Temperature out-of-range */
	ALERT_FANS, /* Temperature doesn't drop after the fans are activated. */
	ALERT_HUMID, /* Humidity out-of-range */
	/* ALERT_LIGHT,  Luminescence too low OR oN/oFF not as scheduled */
	ALERT_FLOAT, /* Water level too low */
	/* ALERT_MOISTURE, Soil Moisture remaining - too soggy, too dry */
	/* ALERT_CLEAN,   Time to clean */
	ALERT_CHANGE, /*  Time to add/remove nutrient solution */
	/* ALERT_TREAT,  Time for prevention treatment */
	/* ALERT_PESTS,   Pest Checker:  Consider temp, humidity, lighting schedule */
	ALERT_SYSTEM
};

typedef struct Alert
{
	ALERT_TYPE type;		// The type of the alert (ex: ALERT_TEMP)
	uint8_t freq;		// How frequently should the alert be sent?
	bool state;		// Alert is ON / OFF
	bool triggered;  // flag is true if Alert has been activated
	unsigned long timestamp;
	struct Alert *next;
} Alert;

DataObject rxAlertObject		  = { /*empty*/ };
uint8_t totalAlerts = 6;

const char* alert_message[] = {
	"Too Cold! Req Heater",
	"Too Hot! Need Ventilation",
	"Humidity Out-of-Range",
	"Water Level Low",
	"Time to Adjust",
"Something is Wrong!" };

Alert Alert_Temp     = { ALERT_TEMP, 0, true, false, DEFAULT_TIME, NULL };
Alert Alert_Fans     = { ALERT_FANS, 0, true, false, DEFAULT_TIME, &Alert_Temp };
Alert Alert_Humid    = { ALERT_HUMID, 0, true, false, DEFAULT_TIME, &Alert_Fans };
//Alert Alert_Light    = { ALERT_LIGHT, 0, true, false, DEFAULT_TIME, &Alert_Humid };
Alert Alert_Float    = { ALERT_FLOAT, 0, true, false,  DEFAULT_TIME, &Alert_Humid };
//Alert Alert_Moist    = { ALERT_MOISTURE, 1, true, false,  DEFAULT_TIME, &Alert_Float };
Alert Alert_Change   = { ALERT_CHANGE, 7, true, false, DEFAULT_TIME, &Alert_Float };
Alert Alert_System  = { ALERT_SYSTEM, 0, false, false, DEFAULT_TIME, &Alert_Change };
/************************************************************************/

bool txAlertData( Alert *alert, const byte SEND_TO_ADDRESS );
void printAlerts( Alert *alert );
bool SyncAlert(DataObject *alert);

void setDataObject(DataObject *dataObject, Alert *alert)
{
	// 	DataObject dataObject;
	dataObject->object = 'x';
	dataObject->node_address = 0;
	dataObject->timestamp = (unsigned long) alert->timestamp;
	dataObject->type = (byte) alert->type;
	dataObject->freq = (uint8_t) alert->freq;
	dataObject->value = 0;
	dataObject->ready = false;
	dataObject->triggered = (bool) alert->triggered;
	dataObject->state = (bool) alert->state;
}

bool txAlertData( Alert *alert, const byte SEND_TO_ADDRESS )
{
	bool r = false;
	for (; alert != NULL; alert = alert->next )
	{
		setDataObject( &txDataObject, alert );
		Wire.beginTransmission (SEND_TO_ADDRESS);
		Wire.write ((byte *) &txDataObject, sizeof(txDataObject));
		uint8_t result = Wire.endTransmission ();
		if (result == 0 ) { r = true; }
		// else { errorHandler( result ); break; if ( DEBUG ) { Serial.println(F(" on txAlert")); } }
		delay(10);
		//printDataObject( &txDataObject );
	}
	return r;
}


void printAlerts( Alert *alert ) {
	if ( DEBUG )
	{
		for (; alert != NULL; alert = alert->next )
		{
			// Don't bother printing unless alert triggered
			//			if ( alert->triggered ) {
			Serial.print(F("Alert: "));
			Serial.print(alert->type);
			Serial.print(F(" t: "));
			Serial.print(alert->triggered);
			Serial.print(F(" : "));
			Serial.print(alert->timestamp);
			Serial.print(F(" msg: "));
			Serial.print(alert_message[alert->type]);
			Serial.println("");
			//			}
		}
	}
}

bool SyncAlert(DataObject *alert) {
	bool r = false;
	
	Alert * _alert = &Alert_System;
	for (; _alert != NULL; _alert = _alert->next )
	{
		if ( _alert->type == alert->type )
		{
			_alert->state = alert->state;
			if ( ( alert->timestamp > DEFAULT_TIME ) && ( _alert->timestamp < alert->timestamp ) )
			{ _alert->triggered = alert->triggered; }
			_alert->timestamp = alert->timestamp;
			r = true;
			break;
		}
	}
	return r;
}


#else
#error This code is only for use on Arduino.
#endif // ARDUINO
#endif // __ALERTS_H__
