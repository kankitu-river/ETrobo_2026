#include "app.h"
#include "LineTracer.h"
#include "TraceLogger.h"
#include "Odometry.h"
#include "LogReplay.h"

#include <spike/pup/motor.h>
#include <spike/pup/colorsensor.h>

/* 関数プロトタイプ宣言 */
static int16_t steering_amount_calculation(void);
static void motor_drive_control(int16_t);

static pup_motor_t *fg_left_motor;
static pup_motor_t *fg_right_motor;
static pup_device_t *fg_color_sensor;
static int32_t fg_reflection;
static int fg_left_motor_power;
static int fg_right_motor_power;
static int fg_base_speed;
static int fg_straight_mode;

void LineTracer_Configure(pbio_port_id_t left_motor_port, pbio_port_id_t right_motor_port, pbio_port_id_t color_sensor_port)
{

  /* センサー入力ポートの設定 */
  fg_color_sensor = pup_color_sensor_get_device(color_sensor_port);
  fg_left_motor   = pup_motor_get_device(left_motor_port);
  fg_right_motor   = pup_motor_get_device(right_motor_port);  

  pup_motor_setup(fg_left_motor,PUP_DIRECTION_COUNTERCLOCKWISE,true);
  pup_motor_setup(fg_right_motor,PUP_DIRECTION_CLOCKWISE,true);

  fg_base_speed    = BASE_SPEED;
  fg_straight_mode = 0;
}

void LineTracer_Stop(void)//走行停止関数
{
  pup_motor_set_power(fg_left_motor, 0);
  pup_motor_set_power(fg_right_motor, 0);
}

/* ライントレースタスク(100msec周期で関数コールされる) */
void tracer_task(intptr_t unused) {

    int16_t steering_amount;
    int32_t left_count, right_count;

    /* ステアリング操舵量の計算 */
    steering_amount = steering_amount_calculation();

    /* 走行モータ制御 */
    motor_drive_control(steering_amount);

    /* エンコーダ取得・オドメトリ更新 (100msec毎) */
    left_count  = pup_motor_get_count(fg_left_motor);
    right_count = pup_motor_get_count(fg_right_motor);
    Odometry_Update(left_count, right_count);

    /* 500msecごとに走行ログをRAMへ保存 */
    TraceLogger_Record(fg_reflection, steering_amount,
                       (int16_t)fg_left_motor_power,
                       (int16_t)fg_right_motor_power,
                       left_count, right_count);

    /* タスク終了 */
    ext_tsk();
}

void LineTracer_SetSpeed(int speed)    { fg_base_speed    = speed; }
void LineTracer_SetStraight(void)      { fg_straight_mode = 1; }
void LineTracer_SetLineTrace(void)     { fg_straight_mode = 0; }

/* ステアリング操舵量の計算 */
static int16_t steering_amount_calculation(void){

    uint16_t  target_brightness; /* 目標輝度値 */
    float     diff_brightness;   /* 目標輝度との差分値 */
    int16_t   steering_amount;   /* ステアリング操舵量 */
    /* 目標輝度値の計算 */
    target_brightness = (WHITE_BRIGHTNESS + BLACK_BRIGHTNESS) / 2;

    /* カラーセンサ値の取得 */
    fg_reflection = pup_color_sensor_reflection(fg_color_sensor);

    /* 目標輝度値とカラーセンサ値の差分を計算 */
    diff_brightness = (float)(target_brightness - fg_reflection);

    /* ステアリング操舵量を計算 */
    steering_amount = (int16_t)(diff_brightness * STEERING_COEF);

    return steering_amount;
}

/* 走行モータ制御 */
static void motor_drive_control(int16_t steering_amount){

    /* 直進モード時はステアリング操舵量を0にする */
    if (fg_straight_mode) { steering_amount = 0; }

    fg_left_motor_power  = (int)(fg_base_speed + (steering_amount * LEFT_EDGE));
    fg_right_motor_power = (int)(fg_base_speed - (steering_amount * LEFT_EDGE));

    /* 左右モータ駆動パワーの設定 */
    pup_motor_set_power(fg_left_motor, fg_left_motor_power);
    pup_motor_set_power(fg_right_motor, fg_right_motor_power);

    return;
}
