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
#include <unistd.h>
#include <time.h>

#include <pthread.h>

#include "./lib_weather/lib_weather.h"

//------------------------------------------------------------------------------
#define MELO_TTS_PATH       "/root/MeloTTS.lib/"
#define AUDIO_THREAD_PATH   MELO_TTS_PATH

#define TODAY_TEXT          MELO_TTS_PATH"today.txt"
#define TIME_TEXT           MELO_TTS_PATH"time.txt"
#define WEATHER_TEXT        MELO_TTS_PATH"weather.txt"

#define STR_PATH_LENGTH     128
#define MAKE_AUDIO_WAIT     200

//------------------------------------------------------------------------------
const char *PlayList[] = {
    "today",
    "time",
    "weather",
};

enum { false = 0, true } bool;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// thread control variable
//------------------------------------------------------------------------------
volatile int PlayAudioEnable = 0, MakeAudioEnable = 0;
pthread_t AudioThread;

void *play_audio_thread_func (void *arg)
{
    const char *fname = (const char *)arg;

    FILE *fp;
    char cmd [STR_PATH_LENGTH *2];

    PlayAudioEnable = 1;

    memset  (cmd, 0, sizeof(cmd));
    sprintf (cmd, "aplay -Dplughw:0,2 %s%s.wav && sync", AUDIO_THREAD_PATH, fname);

    if ((fp = popen (cmd, "w")) != NULL)
        pclose(fp);

    PlayAudioEnable = 0;    usleep (100 *1000);

    return arg;
}

//------------------------------------------------------------------------------
void play_audio_thread_stop (void)
{
    FILE *fp;
    const char *cmd = "ps ax | grep aplay | awk '{print $1}' | xargs kill";

    if ((fp = popen (cmd, "w")) != NULL)
        pclose(fp);

    PlayAudioEnable = 0;    usleep (100 *1000);
}

//------------------------------------------------------------------------------
int play_audio (const char *fname, int wait_sec)
{
    while (PlayAudioEnable)     play_audio_thread_stop ();

    pthread_create (&AudioThread, NULL, play_audio_thread_func, (void *)fname);
    sleep (1);

    while (wait_sec-- && PlayAudioEnable)   {
        printf ("%s %s : wait %d sec\r\n", __func__, fname, wait_sec);  fflush(stdout);
        sleep (1);
    }

    while (PlayAudioEnable)    play_audio_thread_stop ();

    return wait_sec ? true : false;
}

//------------------------------------------------------------------------------
#define DOCKER_CMD  "docker run --rm --network=host -it -v %s:/app melotts -i %s.txt -o %s.wav -l kr -s 1.0"

void *make_audio_thread_func (void *arg)
{
    const char *fname = (const char *)arg;

    FILE *fp;
    char cmd [STR_PATH_LENGTH *2];

    MakeAudioEnable = 1;

    memset  (cmd, 0, sizeof(cmd));
    sprintf (cmd, DOCKER_CMD, MELO_TTS_PATH, fname, fname);

    if ((fp = popen (cmd, "r")) != NULL){
        memset(cmd, 0, sizeof(cmd));
        while (fgets(cmd, sizeof(cmd), fp)) {
            if (strstr(cmd, "mk_speech complete") != NULL) {
                printf ("\r\n%s : Docker mk_sppech.py complete!\r\n", __func__);
                break;
            }
        }
        pclose(fp);
    }

    MakeAudioEnable = 0;    usleep (100 *1000);

    return arg;
}

//------------------------------------------------------------------------------
void make_audio_thread_stop (void)
{
    FILE *fp;
    const char *cmd = "ps ax | grep mk_speech.py | awk '{print $1}' | xargs kill";

    if ((fp = popen (cmd, "w")) != NULL)
        pclose(fp);

    MakeAudioEnable = 0;    usleep (100 *1000);
}

//------------------------------------------------------------------------------
int make_audio (const char *fname, int wait_sec)
{
    while (MakeAudioEnable)     make_audio_thread_stop ();

    pthread_create (&AudioThread, NULL, make_audio_thread_func, (void *)fname);
    sleep (1);

    while (wait_sec-- && MakeAudioEnable)   {
        printf ("%s %s : wait %d sec\r\n", __func__, fname, wait_sec);  fflush(stdout);
        sleep (1);
    }

    while (MakeAudioEnable)    make_audio_thread_stop ();

    return wait_sec ? true : false;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int create_today_txt (void)
{
    FILE *fp;

    if ((fp = fopen (TODAY_TEXT, "wt")) != NULL) {
        fprintf (fp, "오늘은 %s년 ", date_to_kor (eDAY_YEAR, NULL));
        fprintf (fp, "%s월 ", date_to_kor (eDAY_MONTH, NULL));
        fprintf (fp, "%s일 ", date_to_kor (eDAY_DAY, NULL));
        fprintf (fp, "%s요일 입니다.\n", date_to_kor (eDAY_W_DAY, NULL));
        fclose  (fp);

        printf ("오늘은 %s년 ", date_to_kor (eDAY_YEAR, NULL));
        printf ("%s월 ", date_to_kor (eDAY_MONTH, NULL));
        printf ("%s일 ", date_to_kor (eDAY_DAY, NULL));
        printf ("%s요일 입니다.\n", date_to_kor (eDAY_W_DAY, NULL));

        return make_audio("today", MAKE_AUDIO_WAIT) ? true : false;
    } else {
        fprintf (stderr, "Can't create a file(%s:%s).\n", __func__, TODAY_TEXT);
        return false;
    }
}

//------------------------------------------------------------------------------
int create_time_txt (void)
{
    FILE *fp;

    if ((fp = fopen (TIME_TEXT, "wt")) != NULL) {
        fprintf (fp, "현재시간은 %s ", date_to_kor (eDAY_AM_PM, NULL));
        fprintf (fp, "%s시 ", date_to_kor (eDAY_HOUR, NULL));
        fprintf (fp, "%s분 입니다.\n", date_to_kor (eDAY_MIN, NULL));
        fclose  (fp);

        printf ("현재시간은 %s ", date_to_kor (eDAY_AM_PM, NULL));
        printf ("%s시 ", date_to_kor (eDAY_HOUR, NULL));
        printf ("%s분 입니다.\n", date_to_kor (eDAY_MIN, NULL));

        return make_audio("time", MAKE_AUDIO_WAIT) ? true : false;
    } else {
        fprintf (stderr, "Can't create a file(%s:%s).\n", __func__, TIME_TEXT);
        return false;
    }
}

//------------------------------------------------------------------------------
int create_weather_txt (const char *cur_lobs)
{
    FILE *fp;
    static char prev_lobs[WTTR_DATA_SIZE] = { 0, };
    static double lati = 0, longi = 0;
    char city[WTTR_DATA_SIZE], country[WTTR_DATA_SIZE];

    /* 측정되어진 wttr 좌표 데이터*/
    printf ("Lati : %s, Longi : %s\n", get_wttr_data (eWTTR_LATITUDE), get_wttr_data (eWTTR_LONGITUDE));

    if (strncmp (prev_lobs, cur_lobs, strlen (cur_lobs)) ||
        lati  != atof(get_wttr_data (eWTTR_LATITUDE))    ||
        longi != atof(get_wttr_data (eWTTR_LONGITUDE)))    {

        strncpy (prev_lobs, cur_lobs, strlen (cur_lobs));
        lati  = atof(get_wttr_data (eWTTR_LATITUDE));
        longi = atof(get_wttr_data (eWTTR_LONGITUDE));

        if ((fp = fopen (WEATHER_TEXT, "wt")) != NULL) {
            /* 좌표기준으로 위치 검색 */
            get_location_json (lati, longi, city, country, 1);

            /*
                날씨 재생 Format
                대한민국 경기도 날씨를 알려드립니다.
                현재 날씨는 맑은 입니다.
                온도는 27도 체감온도는 26도 이며 습도는 65퍼센트 바람은 동남쪽으로 시속 1킬로미터로 붑니다.
                강수량은 0밀리미터 입니다.
            */
            fprintf (fp, "%s %s 날씨를 알려드립니다.\n", country, city);
            fprintf (fp, "현재 날씨는 %s 입니다.\n", translate_weather_code( get_wttr_data (eWTTR_W_CODE), 1 ));
            fprintf (fp, "온도는 %s도, ", int_to_kor (atoi( get_wttr_data (eWTTR_TEMP))));
            fprintf (fp, "체감온도는 %s도 이며, ", int_to_kor (atoi( get_wttr_data (eWTTR_TEMP_FEEL))));
            fprintf (fp, "습도는 %s퍼센트, ", int_to_kor (atoi( get_wttr_data (eWTTR_HUMIDUTY))));
            fprintf (fp, "바람은 %s쪽으로, ", translate_wind_degree ( get_wttr_data (eWTTR_WIND_DIR), 1 ));
            fprintf (fp, "시속 %s킬로미터로 붑니다.\n",  int_to_kor (atoi (get_wttr_data (eWTTR_WIND_SPEED))));
            if (atoi(get_wttr_data (eWTTR_PRECIPI)) > 0)
                fprintf (fp, "강수량은 %s밀리미터 입니다.\n", int_to_kor (atoi( get_wttr_data (eWTTR_PRECIPI))));

            fclose  (fp);

            return make_audio("weather", MAKE_AUDIO_WAIT) ? true : false;
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
            create_weather_txt (get_wttr_data (eWTTR_LOBS_DATE));

            play_audio ("today", 10);
            play_audio ("time", 10);
            play_audio ("weather", 30);
        }
    }
    return 0;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

