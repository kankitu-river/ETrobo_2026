#ifdef __cplusplus
extern "C" {
#endif

/* 下記の項目は各ロボットに合わせて変えること */

/* カラーセンサの輝度設定 */
#define WHITE_BRIGHTNESS  (80)
#define BLACK_BRIGHTNESS  (10)

/* PID制御ゲイン */
#define KP  (0.4F)   /* 比例ゲイン */
#define KI  (0.01F)  /* 積分ゲイン */
#define KD  (0.1F)   /* 微分ゲイン */

/* 積分値の上限（ワインドアップ防止） */
#define INTEGRAL_MAX  (500.0F)

/* 走行基準スピード */
#define BASE_SPEED    (60)

/* ライントレースエッジ切り替え（左エッジ=1, 右エッジ=-1） */
#define LEFT_EDGE     (1)
#define RIGHT_EDGE    (-1)
#define TRACE_EDGE    LEFT_EDGE

#include <pbio/port.h>

extern void LineTracer_Configure(pbio_port_id_t left_motor_port, pbio_port_id_t right_motor_port, pbio_port_id_t color_sensor_port);

#ifdef __cplusplus
}
#endif
