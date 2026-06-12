#include "app.h"
#include "LineTracer.h"
#include <stdio.h>

#include <spike/pup/motor.h>
#include <spike/pup/colorsensor.h>

/* 関数プロトタイプ宣言 */
static int16_t pid_control(void);
static void motor_drive_control(int16_t steering_amount);

static pup_motor_t  *fg_left_motor;
static pup_motor_t  *fg_right_motor;
static pup_device_t *fg_color_sensor;

/* PID制御用内部変数 */
static float fg_integral;
static float fg_prev_error;

void LineTracer_Configure(pbio_port_id_t left_motor_port, pbio_port_id_t right_motor_port, pbio_port_id_t color_sensor_port)
{
    fg_color_sensor = pup_color_sensor_get_device(color_sensor_port);
    fg_left_motor   = pup_motor_get_device(left_motor_port);
    fg_right_motor  = pup_motor_get_device(right_motor_port);

    pup_motor_setup(fg_left_motor,  PUP_DIRECTION_COUNTERCLOCKWISE, true);
    pup_motor_setup(fg_right_motor, PUP_DIRECTION_CLOCKWISE,        true);

    /* PID状態の初期化 */
    fg_integral   = 0.0F;
    fg_prev_error = 0.0F;
}

/* ライントレースタスク(100msec周期で関数コールされる) */
void tracer_task(intptr_t unused)
{
    int16_t steering_amount = pid_control();
    motor_drive_control(steering_amount);
    ext_tsk();
}

/* PID制御によるステアリング操舵量の計算 */
static int16_t pid_control(void)
{
    uint16_t target = (WHITE_BRIGHTNESS + BLACK_BRIGHTNESS) / 2;
    float    error  = (float)target - (float)pup_color_sensor_reflection(fg_color_sensor);

    /* 積分項（ワインドアップ防止付き） */
    fg_integral += error;
    if      (fg_integral >  INTEGRAL_MAX) fg_integral =  INTEGRAL_MAX;
    else if (fg_integral < -INTEGRAL_MAX) fg_integral = -INTEGRAL_MAX;

    /* 微分項 */
    float derivative = error - fg_prev_error;
    fg_prev_error    = error;

    float steering = (KP * error) + (KI * fg_integral) + (KD * derivative);

    return (int16_t)(steering * TRACE_EDGE);
}

/* 走行モータ制御 */
static void motor_drive_control(int16_t steering_amount)
{
    int left_motor_power  = BASE_SPEED + (int)steering_amount;
    int right_motor_power = BASE_SPEED - (int)steering_amount;

    pup_motor_set_power(fg_left_motor,  left_motor_power);
    pup_motor_set_power(fg_right_motor, right_motor_power);
}
