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
        if (((wait_sec % 10) == 0) || (wait_sec < 10)) {
            printf ("%s %s : wait %d sec\r\n", __func__, fname, wait_sec);  fflush(stdout);
        }
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
        if (((wait_sec % 10) == 0) || (wait_sec < 10)) {
            printf ("%s %s : wait %d sec\r\n", __func__, fname, wait_sec);  fflush(stdout);
        }
        sleep (1);
    }

    while (MakeAudioEnable)    make_audio_thread_stop ();

    return wait_sec ? true : false;
}

//------------------------------------------------------------------------------
void get_local_date (struct tm *get_lt)
{
    time_t t = time(NULL);
    struct tm *lt;

    setenv("TZ", "Asia/Seoul", 1);  // 타임존 설정
    lt = localtime(&t);

    // struct 내용 복사.
    memcpy (get_lt, lt, sizeof(struct tm));
}

//------------------------------------------------------------------------------
int create_today_txt (void)
{
    FILE *fp;
    struct tm lt;

    /* 오늘이 몇 일인지(0~365) */
    static int yday = -1;

    /* 1번만 생성하기 위하여 현재 시간을 읽어와 날짜가 바뀌었는지 확인 */
    get_local_date(&lt);

    if (yday != -1) {
        if (yday == lt.tm_yday) {
            /* 모든 파일이 있는 경우에는 기존에 생성되었던 파일을 사용함. */
            if (!access (MELO_TTS_PATH"today.txt", F_OK) && !access (MELO_TTS_PATH"today.wav", F_OK))
                return true;

            printf ("%stoday.* fine not found\n", MELO_TTS_PATH);
        }
    }
    yday = lt.tm_yday;

    if ((fp = fopen (TODAY_TEXT, "wt")) != NULL) {
        fprintf (fp, "오늘은 %s년 ", date_to_kor (eDAY_YEAR, NULL));
        fprintf (fp, "%s월 ", date_to_kor (eDAY_MONTH, NULL));
        fprintf (fp, "%s일 ", date_to_kor (eDAY_DAY, NULL));
        fprintf (fp, "%s요일 입니다.\n", date_to_kor (eDAY_W_DAY, NULL));
        fclose  (fp);

        printf ("\n%s\n", __func__);
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
    struct tm lt;

    /* 현재 시간을 읽어와 분이 0인 경우 처리 */
    get_local_date(&lt);

    if ((fp = fopen (TIME_TEXT, "wt")) != NULL) {
        fprintf (fp, "현재시간은 %s ", date_to_kor (eDAY_AM_PM, (void *)&lt));
        fprintf (fp, "%s시 ", date_to_kor (eDAY_HOUR, (void *)&lt));
        if (lt.tm_min != 0)
            fprintf (fp, "%s분 ", date_to_kor (eDAY_MIN, (void *)&lt));
        fprintf (fp, "%s\n", "입니다.");
        fclose  (fp);

        printf ("\n%s\n", __func__);
        printf ("현재시간은 %s ", date_to_kor (eDAY_AM_PM, (void *)&lt));
        printf ("%s시 ", date_to_kor (eDAY_HOUR, (void *)&lt));
        if (lt.tm_min != 0)
            printf ("%s분 입니다.\n", date_to_kor (eDAY_MIN, (void *)&lt));
        else
            printf ("%s\n", "입니다.");

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
    static double lati = -1, longi = -1;
    char city[WTTR_DATA_SIZE], country[WTTR_DATA_SIZE];

    /* 측정되어진 wttr 좌표 데이터*/
    printf ("wttr.in DATA = Lati : %s, Longi : %s, lobs : %s\n",
        get_wttr_data (eWTTR_LATITUDE), get_wttr_data (eWTTR_LONGITUDE), get_wttr_data (eWTTR_LOBS_DATE));

    printf ("Local DATA   = Lati : %f, Longi : %f, cur_lobs : %s, prev_lobs : %s\n",
        lati, longi, cur_lobs, prev_lobs);

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

           printf ("\n%s\n", __func__);
           printf ("%s %s 날씨를 알려드립니다.\n", country, city);
           printf ("현재 날씨는 %s 입니다.\n", translate_weather_code( get_wttr_data (eWTTR_W_CODE), 1 ));
           printf ("온도는 %s도, ", int_to_kor (atoi( get_wttr_data (eWTTR_TEMP))));
           printf ("체감온도는 %s도 이며, ", int_to_kor (atoi( get_wttr_data (eWTTR_TEMP_FEEL))));
           printf ("습도는 %s퍼센트, ", int_to_kor (atoi( get_wttr_data (eWTTR_HUMIDUTY))));
           printf ("바람은 %s쪽으로, ", translate_wind_degree ( get_wttr_data (eWTTR_WIND_DIR), 1 ));
           printf ("시속 %s킬로미터로 붑니다.\n",  int_to_kor (atoi (get_wttr_data (eWTTR_WIND_SPEED))));
           if (atoi(get_wttr_data (eWTTR_PRECIPI)) > 0)
               printf ("강수량은 %s밀리미터 입니다.\n", int_to_kor (atoi( get_wttr_data (eWTTR_PRECIPI))));

           return make_audio("weather", MAKE_AUDIO_WAIT) ? true : false;
        }
    }
    return false;
}

//------------------------------------------------------------------------------
void weather_info (void)
{
    struct tm t;

    get_wttr_date (get_wttr_data (eWTTR_LOBS_DATE), &t);

    printf ("Local OBS : %s\n", get_wttr_data (eWTTR_LOBS_DATE));
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

    printf ("\r\n ********** PLAY WEATHER AUDIO(KR) START **********\r\n");
    play_audio ("today", 10);
    play_audio ("time", 10);
    play_audio ("weather", 30);
    printf ("\r\n ********** PLAY WEATHER AUDIO(KR) END **********\r\n");
}

//------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    struct tm t;
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
        weather_info ();

        while (1) {

            get_local_date (&t);

            /* update every 10 min */
//            if ((t.tm_min % 10) == 0) {
                printf ("location : %s\r\n", location);
                if (update_weather_data (location))
                    weather_info();
//            }

            sleep (1);
        }
    }
    return 0;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

