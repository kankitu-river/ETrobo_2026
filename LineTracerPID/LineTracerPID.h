#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 *  LineTracerPID  ―  PID制御によるライントレース
 *
 *  ETロボコン技術教育テキスト「6.6 フィードバック制御」に基づく実装。
 *    ・6.6.1 オンオフ制御     … LineTracerSimple.c が対応
 *    ・6.6.2 比例制御(P制御)   … 偏差に比例して操舵量を決める
 *    ・6.6.3 PID制御           … P制御の弱点(オフセット/行き過ぎ)を
 *                                 I部(積分)・D部(微分)で補う ← 本ファイル
 *
 *  偏差         e        = 目標輝度 - 反射光量
 *  操舵量(MV)   steering = Kp*e + Ki*Σe + Kd*(e - e_prev)
 *                          └P部┘ └─I部─┘ └────D部────┘
 *
 *  下記の値は各ロボット・コースに合わせて調整すること。
 * ========================================================================= */

/* --- カラーセンサの輝度設定（反射光量の実測値で調整） --- */
#define WHITE_BRIGHTNESS    (75)     /* 白マット上の反射光量 */
#define BLACK_BRIGHTNESS    (15)     /* 黒ライン上の反射光量 */
/* 目標輝度 = 白と黒の境界 = 白黒の中間値（ライントレースの目標値SP） */

/* --- PIDゲイン ---
 *  Kp : 大きいほど反応が機敏。大きすぎると振動（ハンチング）。
 *  Ki : 比例制御で残るオフセット（定常偏差）を時間をかけて消す。
 *       大きすぎると積分の溜まり過ぎでオーバーシュート。
 *  Kd : 偏差の変化（カーブ進入など急変）を先読みして行き過ぎを抑える。
 *       大きすぎるとノイズに敏感になり振動。
 *  まず Kp だけで走らせ → Kd で振動を抑え → 必要なら Ki でオフセット除去、
 *  の順で調整するとよい。
 */
#define KP                  (0.3F)   /* 比例ゲイン (P部) */
#define KI                  (0.0F)   /* 積分ゲイン (I部) */
#define KD                  (0.4F)   /* 微分ゲイン (D部) */

/* --- 積分の暴走防止（アンチワインドアップ） --- */
#define INTEGRAL_MAX        (1000.0F)/* 積分値の絶対値上限 */

/* --- 走行スピード --- */
#define BASE_SPEED          (50)     /* 直線時の基準スピード */
#define CURVE_SPEED         (25)     /* カーブ時のスピード（減速） */
#define CURVE_THRESHOLD     (10)     /* |操舵量|がこれを超えたらカーブと判定 */
#define MAX_STEERING        (30)     /* 操舵量の上限（交差点等の急変を抑制） */

/* --- 色付き物体の検出（彩度が高い区間はPID更新をスキップ） --- */
#define COLOR_SAT_THRESHOLD (30)     /* 彩度がこれ以上なら色付きと判定 */

/* --- ライントレースエッジ（左エッジ:1 / 右エッジ:-1） --- */
#define LEFT_EDGE           (1)
#define RIGHT_EDGE          (-1)
#define TRACE_EDGE          LEFT_EDGE  /* 走行エッジ。右エッジにするなら RIGHT_EDGE */

#include <pbio/port.h>

  extern void LineTracerPID_Configure(pbio_port_id_t left_motor_port,
                                      pbio_port_id_t right_motor_port,
                                      pbio_port_id_t color_sensor_port);

#ifdef __cplusplus
}
#endif
