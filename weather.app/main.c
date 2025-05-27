//------------------------------------------------------------------------------
/**
 * @file main.c
 * @author charles-park (charles.park@hardkernel.com)
 * @brief ODROID-C5 Weather station App.
 * @version 2.0
 * @date 2025-05-14
 *
 * @package apt install libcurl4-openssl-dev libcjson-dev
 *
 * @copyright Copyright (c) 2022
 *
 */
//------------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "./lib_weather/lib_weather.h"

//------------------------------------------------------------------------------
const char *PlayList[] = {
    "today",
    "time",
    "weather",
};

enum { false = 0, true } bool;

//------------------------------------------------------------------------------
int create_today_txt (void)
{
    FILE *fp;

    if ((fp = fopen ("today.txt", "wt")) != NULL) {
        fprintf (fp, "오늘은 %s년 ", date_to_kor (eDAY_YEAR, NULL));
        fprintf (fp, "%s월 ", date_to_kor (eDAY_MONTH, NULL));
        fprintf (fp, "%s일 ", date_to_kor (eDAY_DAY, NULL));
        fprintf (fp, "%s요일 입니다.\n", date_to_kor (eDAY_W_DAY, NULL));
        fclose (fp);

        printf ("오늘은 %s년 ", date_to_kor (eDAY_YEAR, NULL));
        printf ("%s월 ", date_to_kor (eDAY_MONTH, NULL));
        printf ("%s일 ", date_to_kor (eDAY_DAY, NULL));
        printf ("%s요일 입니다.\n", date_to_kor (eDAY_W_DAY, NULL));
        return true;
    } else {
        fprintf (stderr, "Can't create a file(%s).\n", __func__);
        return false;
    }
}

//------------------------------------------------------------------------------
int create_time_txt (void)
{
    FILE *fp;

    if ((fp = fopen ("time.txt", "wt")) != NULL) {
        fprintf (fp, "현재시간은 %s ", date_to_kor (eDAY_AM_PM, NULL));
        fprintf (fp, "%s시 ", date_to_kor (eDAY_HOUR, NULL));
        fprintf (fp, "%s분 입니다.\n", date_to_kor (eDAY_MIN, NULL));
        fclose (fp);

        printf ("현재시간은 %s ", date_to_kor (eDAY_AM_PM, NULL));
        printf ("%s시 ", date_to_kor (eDAY_HOUR, NULL));
        printf ("%s분 입니다.\n", date_to_kor (eDAY_MIN, NULL));
        return true;
    } else {
        fprintf (stderr, "Can't create a file(%s).\n", __func__);
        return false;
    }
}

//------------------------------------------------------------------------------
int create_weather_txt (const char *cur_lobs)
{
    FILE *fp;
    static char prev_lobs[WTTR_DATA_SIZE] = { 0, };
    char city[WTTR_DATA_SIZE], country[WTTR_DATA_SIZE];

    /* 측정되어진 wttr 좌표 데이터*/
    printf ("Lati : %s, Longi : %s\n", get_wttr_data (eWTTR_LATITUDE), get_wttr_data (eWTTR_LONGITUDE));

    if (strncmp (prev_lobs, cur_lobs, strlen (cur_lobs))) {

        if ((fp = fopen ("weather.txt", "wt")) != NULL) {
            strncpy (prev_lobs, cur_lobs, strlen (cur_lobs));
            /* 좌표기준으로 위치 검색 */
            get_location_json (
                atof(get_wttr_data (eWTTR_LATITUDE)),
                atof(get_wttr_data (eWTTR_LONGITUDE)),
                city, country, 1);

            fprintf (fp, "%s %s 현재 날씨를 알려드립니다.\n", country, city);

            fprintf (fp, "현재시간은 %s ", date_to_kor (eDAY_AM_PM, NULL));



            fclose (fp);
            return true;
        }

    }
    return false;
}

//------------------------------------------------------------------------------
int main(int argc, char *argv[]) {

    char location [WTTR_DATA_SIZE];

    memset (location, 0, sizeof(location));

    switch (argc) {
        /* 지역명 (한글, 영어 사용가능) */
        case 2:     sprintf (location, "%s", strlen(argv[1]) ? argv[1] : "");   break;
        /* 위치명 (위도, 경도 입력) */
        case 3:     sprintf (location, "%s,%s", argv[1], argv[2]);              break;
        /* 현 위치 (IP위치 검색) */
        default :   sprintf (location, "%s", "");                               break;
    }

    if (update_weather_data (location)) {
        char city[WTTR_DATA_SIZE], country[WTTR_DATA_SIZE];

        /* 측정되어진 wttr 좌표 데이터*/
        printf ("Lati : %s, Longi : %s\n", get_wttr_data (eWTTR_LATITUDE), get_wttr_data (eWTTR_LONGITUDE));

        /* 좌표기준으로 위치 검색 */
        get_location_json (
            atof(get_wttr_data (eWTTR_LATITUDE)),
            atof(get_wttr_data (eWTTR_LONGITUDE)),
            city, country, 1);

        printf ("Korean : city(%s), country(%s)\n", city, country);

        get_location_json (
            atof(get_wttr_data (eWTTR_LATITUDE)),
            atof(get_wttr_data (eWTTR_LONGITUDE)),
            city, country, 0);

        printf ("English : city(%s), country(%s)\n", city, country);

        char kor_str[WTTR_DATA_SIZE];

        // void date_to_kor_buf (enum eDayItem d_item, void *i_time, char *k_str)
        int_to_kor_buf (atoi(get_wttr_data (eWTTR_TEMP_FEEL)), kor_str);
        printf ("체감온도 : %s도씨\n", kor_str);

        {
            struct tm t;

            get_wttr_date (get_wttr_data (eWTTR_LOBS_DATE), &t);
            // void date_to_kor     (enum eDayItem d_item, void *i_time, char *k_str)
            printf ("측정시간 : ");
            printf ("%s년 ", date_to_kor (eDAY_YEAR, (void *)&t));
            printf ("%s월 ", date_to_kor (eDAY_MONTH, (void *)&t));
            printf ("%s일 ", date_to_kor (eDAY_DAY, (void *)&t));
            printf ("%s요일 ", date_to_kor (eDAY_W_DAY, (void *)&t));
            printf ("%s ", date_to_kor (eDAY_AM_PM, (void *)&t));
            printf ("%s시 ", date_to_kor (eDAY_HOUR, (void *)&t));
            printf ("%s분\n", date_to_kor (eDAY_MIN, (void *)&t));

            printf ("현재시간 : ");
            printf ("%s년 ", date_to_kor (eDAY_YEAR, NULL));
            printf ("%s월 ", date_to_kor (eDAY_MONTH, NULL));
            printf ("%s일 ", date_to_kor (eDAY_DAY, NULL));
            printf ("%s요일 ", date_to_kor (eDAY_W_DAY, NULL));
            printf ("%s ", date_to_kor (eDAY_AM_PM, NULL));
            printf ("%s시 ", date_to_kor (eDAY_HOUR, NULL));
            printf ("%s분\n", date_to_kor (eDAY_MIN, NULL));

            create_today_txt ();
            create_time_txt  ();
        }
    }
    return 0;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

