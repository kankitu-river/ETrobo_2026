#include "LogReplay.h"

/* ログから生成したステアリング値 (500msごと) */
static const int16_t fg_steering_log[] = {
      -4,     -12,       0,      -2,      -2,      -2,      -2,      -2,      -2,      -2,
      -2,      -2,      -7,      -9,      -9,     -11,      -8,      -9,      -9,      -8,
      -4,      -2,      -2,      -2,      -2,      -1,      -1,      -1,      -2,      -1,
      -2,      -2,      -4,      -6,      -8,      -6,      -6,      -8,      -7,      -7,
      -8,      -6,      -7,      -7,      -2,      -3,      -1,      -2,      -1,      -4,
      -5,      -4,      -5,      -4,      -4,      -6,      -6,      -4,      -4,      -3,
      -4,      -3,      -3,      -4,      -4,      -3,      -4,      -4,      -3,      -3,
      -2,       0,      -2,      -2,      -2,      -2,      -8,      -7,      -8,      -8,
      -9,      -9,     -10,      -9,      -2,      -2,       0,      -1,       0,      -2,
      -3,       0,      -2,      -2,      -2,       0,      -2,      -1,      -4,      -9,
     -10,     -10,      -8,      -9,      -8,      -7,      -2,      -2,      -2,       0,
       0,      -1,      -1,       0,       0,      -1,      -1,      -2,      -5,      -5,
      -5,      -7,      -6,      -6,      -7,      -7,      -6,      -6,      -5,      -2,
      -2,       0,       0,      -1,      -4,      -3,      -5,      -4,      -2,      -2,
      -4,      -3,      -4,      -3,      -3,      -3,      -3,      -2,      -3,      -2,
      -4,      -3,      -3,       0,       0,       1,       0,       4,      -1,      -3,
      -6,      -8,      -8,      -7,     -10,      -8,      -9,      -2,       0,      -1,
       0,       0,       0,      -1,       0,       0,       0,
};

#define LOG_ENTRY_COUNT (177U)
#define CYCLES_PER_ENTRY (5U)   /* 1エントリあたりの制御周期数(100ms×5=500ms) */
#define TOTAL_LAPS       (2U)   /* 周回数 */

static uint32_t fg_cycle;
static uint32_t fg_entry;
static uint32_t fg_lap;
static int      fg_finished;

void LogReplay_Init(void)
{
    fg_cycle    = 0U;
    fg_entry    = 0U;
    fg_lap      = 0U;
    fg_finished = 0;
}

int LogReplay_IsFinished(void)
{
    return fg_finished;
}

int16_t LogReplay_GetSteering(void)
{
    if (fg_finished) { return 0; }

    int16_t steering = fg_steering_log[fg_entry];

    fg_cycle++;
    if (fg_cycle >= CYCLES_PER_ENTRY) {
        fg_cycle = 0U;
        fg_entry++;
        if (fg_entry >= LOG_ENTRY_COUNT) {
            fg_entry = 0U;
            fg_lap++;
            if (fg_lap >= TOTAL_LAPS) {
                fg_finished = 1;
            }
        }
    }

    return steering;
}

uint32_t LogReplay_GetLap(void)
{
    return fg_lap;
}
