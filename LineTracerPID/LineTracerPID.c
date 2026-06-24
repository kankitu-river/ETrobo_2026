/* =========================================================================
 *  LineTracerPID.c  ―  PID制御によるライントレース
 *
 *  ETロボコン技術教育テキスト「6.6.3 PID制御」に基づく実装。
 *  比例制御(P)に積分(I)と微分(D)を加え、オフセットの除去と
 *  カーブ進入時の行き過ぎ（オーバーシュート）の抑制を行う。
 * ========================================================================= */
#include "app.h"
#include "LineTracerPID.h"
#include <stdio.h>

#include <spike/pup/motor.h>
#include <spike/pup/colorsensor.h>

/* 関数プロトタイプ宣言 */
static int16_t steering_amount_calculation(void);
static void    motor_drive_control(int16_t steering_amount);

/* デバイスハンドル */
static pup_motor_t  *fg_left_motor;
static pup_motor_t  *fg_right_motor;
static pup_device_t *fg_color_sensor;

/* -------------------------------------------------------------------------
 *  初期設定：モータ・カラーセンサのポート割り当て
 * ------------------------------------------------------------------------- */
void LineTracerPID_Configure(pbio_port_id_t left_motor_port,
                             pbio_port_id_t right_motor_port,
                             pbio_port_id_t color_sensor_port)
{
    /* センサー・モータのデバイス取得 */
    fg_color_sensor = pup_color_sensor_get_device(color_sensor_port);
    fg_left_motor   = pup_motor_get_device(left_motor_port);
    fg_right_motor  = pup_motor_get_device(right_motor_port);

    /* モータの回転方向設定（ロボットに合わせて調整） */
    pup_motor_setup(fg_left_motor,  PUP_DIRECTION_COUNTERCLOCKWISE, true);
    pup_motor_setup(fg_right_motor, PUP_DIRECTION_CLOCKWISE,        true);
}

/* -------------------------------------------------------------------------
 *  ライントレースタスク（一定周期で呼び出される）
 * ------------------------------------------------------------------------- */
void tracer_task(intptr_t unused)
{
    int16_t steering_amount;

    /* PID制御で操舵量を計算 */
    steering_amount = steering_amount_calculation();

    /* 操舵量を左右モータのパワーに反映 */
    motor_drive_control(steering_amount);

    ext_tsk();
}

/* -------------------------------------------------------------------------
 *  ステアリング操舵量の計算（PID制御）
 *      steering = Kp*e + Ki*Σe + Kd*(e - e_prev)
 * ------------------------------------------------------------------------- */
static int16_t steering_amount_calculation(void)
{
    /* 周期をまたいで保持する状態量 */
    static float   integral      = 0.0F; /* 偏差の積算値 Σe        (I部) */
    static float   prev_diff     = 0.0F; /* 前回の偏差 e_prev       (D部) */
    static int16_t prev_steering = 0;    /* 前回の操舵量（色検出時に維持） */

    pup_color_hsv_t hsv;
    uint16_t target_brightness;  /* 目標輝度 SP（白黒の境界） */
    int32_t  reflection;         /* 反射光量 PV（現在値） */
    float    diff;               /* 偏差 e = SP - PV */
    float    p_term, i_term, d_term;
    int16_t  steering_amount;

    /* --- 色付き物体の検出 ---
     * 彩度が高い区間（青マーカー等）はライン情報として扱わず、
     * 前回の操舵量を維持してPIDの状態を乱さない。 */
    hsv = pup_color_sensor_hsv(fg_color_sensor, true);
    if (hsv.s > COLOR_SAT_THRESHOLD) {
        return prev_steering;
    }

    /* --- 目標輝度（白と黒の中間 = 境界） --- */
    target_brightness = (WHITE_BRIGHTNESS + BLACK_BRIGHTNESS) / 2;

    /* --- 反射光量の取得と偏差の計算 --- */
    reflection = pup_color_sensor_reflection(fg_color_sensor);
    diff = (float)((int32_t)target_brightness - reflection);

    /* --- I部：偏差を積算（オフセット除去）。暴走防止に上下限を設ける --- */
    integral += diff;
    if (integral >  INTEGRAL_MAX) integral =  INTEGRAL_MAX;
    if (integral < -INTEGRAL_MAX) integral = -INTEGRAL_MAX;

    /* --- D部：偏差の変化量（急変の先読み） --- */
    d_term    = KD * (diff - prev_diff);
    prev_diff = diff;

    /* --- P部・I部 --- */
    p_term = KP * diff;
    i_term = KI * integral;

    /* --- PID合成 --- */
    steering_amount = (int16_t)(p_term + i_term + d_term);

    prev_steering = steering_amount;
    return steering_amount;
}

/* -------------------------------------------------------------------------
 *  走行モータ制御：操舵量を左右モータのパワー差に変換
 * ------------------------------------------------------------------------- */
static void motor_drive_control(int16_t steering_amount)
{
    int left_motor_power, right_motor_power;
    int speed;

    /* 操舵量の上限制限（交差点等での急スパイクを抑える） */
    if (steering_amount >  MAX_STEERING) steering_amount =  MAX_STEERING;
    if (steering_amount < -MAX_STEERING) steering_amount = -MAX_STEERING;

    /* 操舵量が大きい = カーブ → 減速して安定させる */
    if (steering_amount < -CURVE_THRESHOLD || steering_amount > CURVE_THRESHOLD) {
        speed = CURVE_SPEED;
    } else {
        speed = BASE_SPEED;
    }

    /* 左右モータのパワー（TRACE_EDGE でエッジ方向を切り替え） */
    left_motor_power  = speed + (steering_amount * TRACE_EDGE);
    right_motor_power = speed - (steering_amount * TRACE_EDGE);

    pup_motor_set_power(fg_left_motor,  left_motor_power);
    pup_motor_set_power(fg_right_motor, right_motor_power);
}
