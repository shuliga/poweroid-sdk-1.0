//
// Created by SHL on 28.09.2019.
//

#ifndef DATETIME_H
#define DATETIME_H

#include "../lib/DS1307/DS1307.h"

#define TP_SEC  DS1307_SEC
#define TP_MIN  DS1307_MIN
#define TP_HR   DS1307_HR
#define TP_DOW  DS1307_DOW
#define TP_DATE DS1307_DATE
#define TP_MTH  DS1307_MTH
#define TP_YR   DS1307_YR

#define _datetime_merge( M, TP ) (M*10+TP)
#define _datetime_get_idx( TPM ) (TPM % 10)
#define _datetime_get_max( TPM ) ((TPM - TPM % 10) / 10)


extern const char * TIME_FMT;
extern const char * DATE_FMT;

extern char dateString[9];
extern char timeString[9];

extern const char time[5];
extern const char date[5];

typedef enum DateTimePart{
    DT_SEC = _datetime_merge(59,TP_SEC), DT_MIN = _datetime_merge(59,TP_MIN), DT_HR = _datetime_merge(23,TP_HR), DT_DOW = _datetime_merge(7,TP_DOW), DT_DATE = _datetime_merge(31,TP_DATE), DT_MTH = _datetime_merge(12,TP_MTH), DT_YR = _datetime_merge(99,TP_YR)
};

class DateTime{
public:
    DateTime();

    void setTimeFromString(const char time[9]); // Time string format: "hh:mm:ss"
    void setDateFromString(const char date[9]); // Date string format: "dd/MM/yy"
    void getTimeString(char *time);
    void getDateString(char *date);
    uint8_t getDoW();
    void setDoW(uint8_t dow);
    void dialDateTimeString(char *date_or_time, uint8_t time_part_idx, bool decrease, bool isTime, boolean updateClock);

    void screenDateTimePart(char *date_or_time, uint8_t time_part_idx);
};

extern DateTime DATETIME;

#endif //DATETIME_H
