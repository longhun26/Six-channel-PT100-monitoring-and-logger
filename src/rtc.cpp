#include "rtc.h"
#include <RtcDS1302.h>
#include "config.h"
#include "globals.h"




void initRTC() {
    Rtc.Begin();
    Rtc.SetDateTime(RtcDateTime(year, month, day, hours, minutes, seconds));
    RtcDateTime now = Rtc.GetDateTime();
    year = now.Year();
    month = now.Month();
    day = now.Day();
    hours = now.Hour();
    minutes = now.Minute();
    seconds = now.Second();
    Rtc.SetDateTime(RtcDateTime(year, month, day, hours, minutes, seconds));
}