//
// Created by SHL on 28.09.2019.
//

#include "datetime.h"

const char * TIME_FMT = "%02d:%02d:%02d";
const char * DATE_FMT = "%02d/%02d/%02d";

char dateString[9];
char timeString[9];

const char time[5] = "TIME";
const char date[5] = "DATE";


static const DateTimePart timePart[] {DT_HR, DT_MIN, DT_SEC};
static const DateTimePart datePart[] {DT_DATE, DT_MTH, DT_YR};



DateTime::DateTime() {
    RTC.start();
}

void DateTime::setTimeFromString(const char time[9]) {
    char tmp[3];
    tmp[2] = '\0';

    strncpy(tmp, time, 2);
    uint8_t hr = atoi(tmp);

    strncpy(tmp, time + 3, 2);
    uint8_t min = atoi(tmp);

    strncpy(tmp, time + 6, 2);
    uint8_t sec = atoi(tmp);

    RTC.set(DS1307_HR, hr);
    RTC.set(DS1307_MIN, min);
    RTC.set(DS1307_SEC, sec);
}

void DateTime::setDateFromString(const char date[9]) {
    char tmp[3];
    tmp[2] = '\0';

    strncpy(tmp, date, 2);
    int8_t day = atoi(tmp);

    strncpy(tmp, date + 3, 2);
    int8_t month = atoi(tmp);

    strncpy(tmp, date + 6, 2);
    int8_t year = atoi(tmp);

    RTC.set(DS1307_DATE, day);
    RTC.set(DS1307_MTH, month);
    RTC.set(DS1307_YR, year);
};

void DateTime::getTimeString(char *time) {
    sprintf(time, TIME_FMT, RTC.get(DS1307_HR, true), RTC.get(DS1307_MIN, true), RTC.get(DS1307_SEC, true));
}

void DateTime::getDateString(char *date) {
    sprintf(date, DATE_FMT, RTC.get(DS1307_DATE, true), RTC.get(DS1307_MTH, true), RTC.get(DS1307_YR, true) - DS1307_BASE_YR);
}

uint8_t DateTime::getDoW() {
    return RTC.get(DS1307_DOW, true);
}

void DateTime::setDoW(uint8_t dow) {
    return RTC.set(DS1307_DOW, dow);
}

void DateTime::dialDateTimeString(char *date_or_time, uint8_t time_part_idx, bool isTime, bool decrease, boolean updateClock) {
    char tmp[3];
    uint8_t offset = (3 * time_part_idx);
    strncpy(tmp, date_or_time + offset, 2);
    tmp[2] = '\0';
    int8_t val = atoi(tmp);
    DateTimePart  part = isTime ? timePart[time_part_idx] : datePart[time_part_idx];
    uint8_t min = isTime ? 0 : 1;
    val = decrease ? (val > min ? --val : val) : (val < _datetime_get_max(part) ? ++val : val);
    sprintf(tmp, "%02d", val);
    strncpy(date_or_time + offset, tmp, 2);
    if (updateClock){
        RTC.set(_datetime_get_idx(part), val);
    }
}

void DateTime::screenDateTimePart(char *date_or_time, uint8_t time_part_idx){
    for(uint8_t i = 0; i < 3; i++){
        if (i == time_part_idx) {
            strncpy(date_or_time + (i * 3), "__", 2);
        }
    }
}

DateTime DATETIME = DateTime();

