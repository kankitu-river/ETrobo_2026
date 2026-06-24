#include "kernel_cfg.h"
#include "app.h"
#include <stdio.h>
#include <t_syslog.h>

#include <spike/pup/colorsensor.h>

/* センサーポートの定義 */
static const pbio_port_id_t color_sensor_port = PBIO_PORT_ID_E;

/* メインタスク(起動時にのみ関数コールされる) */
void main_task(intptr_t unused) {
  pup_device_t *color_sensor;
  int32_t reflection;

  /* printfはこの環境でUSBに出ないため、カーネルログと同じsyslog経路で出力する */
  syslog(LOG_NOTICE, "=== reflection log start ===");

  color_sensor = pup_color_sensor_get_device(color_sensor_port);

  while (1) {
    reflection = pup_color_sensor_reflection(color_sensor);
    syslog(LOG_NOTICE, "ref=%d", (int)reflection);
    dly_tsk(100 * 1000);
  }
}
