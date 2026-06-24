#include "TraceLogger.h"

#include <kernel.h>
#include <t_syslog.h>

/* 100msec周期のライントレース処理5回につき、1回ログを保存する。 */
#define TRACE_LOG_INTERVAL_CYCLES  (5U)
#define TRACE_LOG_CAPACITY         (400U)
#define TRACE_DUMP_DELAY           (10 * 1000)

typedef struct {
    uint32_t elapsed_ms;//制御開始からの経過時間[ms]
    int32_t reflection;//カラーセンサの輝度値
    int16_t steering;//ステアリング操舵量
    int16_t left_power;//左モータの出力値
    int16_t right_power;//右モータの出力値
} trace_log_t;

static volatile trace_log_t fg_logs[TRACE_LOG_CAPACITY];//ログ保存用の配列
static uint32_t fg_log_count;//保存されているログの件数
static uint32_t fg_control_cycle_count;//制御周期のカウンタ
static SYSTIM fg_start_time;//制御開始時のシステム時刻
static bool_t fg_log_overflow;//ログが容量オーバーしたかどうかのフラグ

void TraceLogger_Start(void)//ログ記録の開始
{
    fg_log_count = 0U;//ログ件数の初期化
    fg_control_cycle_count = 0U;//制御周期カウンタの初期化
    fg_log_overflow = false;//ログオーバーフラグの初期化
    get_tim(&fg_start_time);//制御開始時のシステム時刻を取得
}

void TraceLogger_Record(int32_t reflection, int16_t steering,//ログ記録関数
                        int16_t left_power, int16_t right_power)
{
    SYSTIM current_time;//現在のシステム時刻
    volatile trace_log_t *log_entry;//ログ保存用のポインタ

    fg_control_cycle_count++;//制御周期カウンタをインクリメント
    if (fg_control_cycle_count < TRACE_LOG_INTERVAL_CYCLES) {
        return;
    }
    fg_control_cycle_count = 0U;//カウンタのリセット

    if (fg_log_count >= TRACE_LOG_CAPACITY) {//ログ容量オーバーのチェック
        fg_log_overflow = true;
        return;
    }

    get_tim(&current_time);//現在のシステム時刻を取得
    log_entry = &fg_logs[fg_log_count++];//ログ保存用の構造体を取得
    log_entry->elapsed_ms = (uint32_t)((current_time - fg_start_time) / 1000U);//制御開始からの経過時間を計算して保存
    log_entry->reflection = reflection;//カラーセンサの輝度値を保存
    log_entry->steering = steering;//ステアリング操舵量を保存
    log_entry->left_power = left_power;//左モータの出力値を保存
    log_entry->right_power = right_power;//右モータの出力値を保存
}

void TraceLogger_Dump(void)//ログのダンプ関数
{
    uint32_t i;
    volatile const trace_log_t *log_entry;
    syslog(LOG_NOTICE, "*START TRACE LOG----sample_c5_log----------------------");
    syslog(LOG_NOTICE, "time_ms,ref,steering,left,right");//CSV形式のヘッダを出力
    for (i = 0U; i < fg_log_count; i++) {//保存されているログを順番に出力
        log_entry = &fg_logs[i];//ログのポインタを取得
        syslog(LOG_NOTICE, "%u,%d,%d,%d,%d",//CSV形式でログを出力
               (unsigned int)log_entry->elapsed_ms,
               (int)log_entry->reflection,
               (int)log_entry->steering,
               (int)log_entry->left_power,
               (int)log_entry->right_power);
        dly_tsk(TRACE_DUMP_DELAY);//ログ出力の間に遅延を入れる（USB通信の安定化のため）
    }
    syslog(LOG_NOTICE, "*END TRACE LOG----sample_c5_log----------------------");
    syslog(LOG_NOTICE, "log_end,count=%u,overflow=%d",//ログの終了を示すメッセージとログ件数、オーバーフラグを出力
           (unsigned int)fg_log_count,
           fg_log_overflow ? 1 : 0);
    syslog_fls_log();//ログバッファをフラッシュして出力
}
