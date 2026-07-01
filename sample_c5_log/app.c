#include "kernel_cfg.h"
#include "app.h"
#include <t_syslog.h>
#include "LineTracer.h"
#include "TraceLogger.h"
#include "Odometry.h"

#include <spike/pup/forcesensor.h>
#include <spike/hub/speaker.h>
#include <math.h>

/* ===== コースセクション定義 ===== */
/* 1周の中で距離に応じてスピードと走行モードを切り替える */
typedef struct {
    float dist_mm;  /* このセクションが始まる距離 [mm] */
    int   speed;    /* 走行スピード (0-100) */
    int   straight; /* 1=直進モード, 0=ライントレース */
} section_t;

/* ↓ 実際のコースに合わせて調整してください */
static const section_t sections[] = {
    {    0, 30, 0 },  /*    0mm〜: 通常ライントレース */
    { 2000, 40, 0 },  /* 2000mm〜: スピードアップ    */
    { 5000, 30, 0 },  /* 5000mm〜: スロー             */
    { 6500, 30, 1 },  /* 6500mm〜: 直進               */
};
#define SECTION_COUNT  (sizeof(sections) / sizeof(sections[0]))

/* ===== 周回設定 ===== */
#define TOTAL_LAPS          (2)       /* 周回数 */
#define ONE_LAP_MM          (7184.0f) /* ログから計算した1周の距離 [mm] */
#define LAP_DETECT_RADIUS   (300.0f)  /* スタート地点からこの距離以内で周回検知 [mm] */
#define LAP_MIN_DIST        (3000.0f) /* 最低この距離走ってから周回検知 [mm] */

/* センサーポートの定義 */
static const pbio_port_id_t
  color_sensor_port = PBIO_PORT_ID_E,
  left_motor_port   = PBIO_PORT_ID_B,
  right_motor_port  = PBIO_PORT_ID_A,
  force_sensor_port = PBIO_PORT_ID_D;

static void wait_force_sensor_pressed(pup_device_t *force_sensor)
{
    while (!pup_force_sensor_touched(force_sensor)) { dly_tsk(10 * 1000); }
}

static void wait_force_sensor_released(pup_device_t *force_sensor)
{
    while (pup_force_sensor_touched(force_sensor)) { dly_tsk(10 * 1000); }
}

/* セクション番号を取得 (1周内の距離で判定) */
static int get_section(float lap_dist)
{
    int sec = 0;
    int i;
    for (i = 0; i < (int)SECTION_COUNT; i++) {
        if (lap_dist >= sections[i].dist_mm) { sec = i; }
    }
    return sec;
}

void main_task(intptr_t unused) {
    pup_device_t *force_sensor;

    syslog(LOG_NOTICE, "Press force sensor to start");
    force_sensor = pup_force_sensor_get_device(force_sensor_port);
    wait_force_sensor_pressed(force_sensor);

    LineTracer_Configure(left_motor_port, right_motor_port, color_sensor_port);
    Odometry_Init();
    TraceLogger_Start();
    hub_speaker_set_volume(50);
    syslog(LOG_NOTICE, "Start: %d laps", TOTAL_LAPS);

    sta_cyc(LINE_TRACER_TASK_CYC);

    int last_beep_count = 0;
    int last_section    = -1;
    int lap_count       = 0;
    float lap_dist      = 0.0f; /* 現在の周の走行距離 */
    float prev_total    = 0.0f;

    while (lap_count < TOTAL_LAPS) {
        float total = Odometry_GetDistance();
        float dx    = Odometry_GetX();
        float dy    = Odometry_GetY();

        /* 今周の走行距離を更新 */
        lap_dist += (total - prev_total);
        prev_total = total;

        /* ① セクション切り替え */
        int sec = get_section(lap_dist);
        if (sec != last_section) {
            LineTracer_SetSpeed(sections[sec].speed);
            if (sections[sec].straight) {
                LineTracer_SetStraight();
            } else {
                LineTracer_SetLineTrace();
            }
            syslog(LOG_NOTICE, "Section %d: speed=%d straight=%d",
                   sec, sections[sec].speed, sections[sec].straight);
            last_section = sec;
        }

        /* ② 座標ベース周回検知 */
        float dist_from_start = sqrtf(dx * dx + dy * dy);
        if (lap_dist > LAP_MIN_DIST && dist_from_start < LAP_DETECT_RADIUS) {
            lap_count++;
            syslog(LOG_NOTICE, "Lap %d done (pos: %d,%d mm)",
                   lap_count, (int)dx, (int)dy);
            hub_speaker_play_tone(NOTE_C5, 400);
            /* 次の周のために位置とセクションをリセット */
            Odometry_ResetPosition();
            lap_dist     = 0.0f;
            last_section = -1;
        }

        /* 100mmごとにビープ（走行確認） */
        int beep_count = (int)(total / 100.0f);
        if (beep_count > last_beep_count) {
            hub_speaker_play_tone(NOTE_A4, 60);
            last_beep_count = beep_count;
        }

        dly_tsk(10 * 1000);
    }

    stp_cyc(LINE_TRACER_TASK_CYC);
    ter_tsk(LINE_TRACER_TASK);
    LineTracer_Stop();
    hub_speaker_play_tone(NOTE_C6, 600);
    syslog(LOG_NOTICE, "Finished %d laps. Connect USB and press force sensor to dump log",
           TOTAL_LAPS);

    wait_force_sensor_released(force_sensor);
    wait_force_sensor_pressed(force_sensor);
    TraceLogger_Dump();

    ext_tsk();
}
