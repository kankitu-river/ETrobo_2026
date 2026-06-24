#include "kernel_cfg.h"
#include "app.h"
#include <t_syslog.h>
#include "LineTracer.h"
#include "TraceLogger.h"

#include <spike/pup/forcesensor.h>

/* センサーポートの定義 */
static const pbio_port_id_t
  color_sensor_port    = PBIO_PORT_ID_E,
  left_motor_port      = PBIO_PORT_ID_B,
  right_motor_port     = PBIO_PORT_ID_A,
  force_sensor_port    = PBIO_PORT_ID_D;

static void wait_force_sensor_pressed(pup_device_t *force_sensor)
{
  while (!pup_force_sensor_touched(force_sensor)) {
    dly_tsk(10 * 1000);
  }
}

static void wait_force_sensor_released(pup_device_t *force_sensor)
{
  while (pup_force_sensor_touched(force_sensor)) {
    dly_tsk(10 * 1000);
  }
}

/* メインタスク(起動時にのみ関数コールされる) */
void main_task(intptr_t unused) {
  pup_device_t *force_sensor;

  syslog(LOG_NOTICE, "Press force sensor to start");
  force_sensor = pup_force_sensor_get_device(force_sensor_port);
  wait_force_sensor_pressed(force_sensor);

  /* LineTracerに構成を渡す */
  LineTracer_Configure(left_motor_port,right_motor_port,color_sensor_port);
  TraceLogger_Start();
  syslog(LOG_NOTICE, "Start Line Trace and logging");
    
  /* ライントレースタスクの起動 */
  sta_cyc(LINE_TRACER_TASK_CYC);

  /* 2回目の押下で走行と記録を停止 */
  wait_force_sensor_released(force_sensor);//1回目の押下が離されるのを待つ
  wait_force_sensor_pressed(force_sensor);//2回目の押下を待つ
  stp_cyc(LINE_TRACER_TASK_CYC);//ラインタレースタスクの停止
  ter_tsk(LINE_TRACER_TASK);//ラインタレースタスクの終了
  LineTracer_Stop();//走行停止関数の呼び出し
  syslog(LOG_NOTICE, "Stopped. Connect USB and press force sensor to dump log");//停止のメッセージを出力し、USB接続と3回目の押下を促す

  /* USB接続後、3回目の押下で蓄積ログをまとめて出力 */
  wait_force_sensor_released(force_sensor);
  wait_force_sensor_pressed(force_sensor);
  TraceLogger_Dump();

  /* タスク終了 */
  ext_tsk();
}
