#ifndef ODOMETRY_H
#define ODOMETRY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define WHEEL_RADIUS_MM  28.0f   /* 車輪半径 [mm] (直径56mm) */
#define WHEEL_BASE_MM    95.5f   /* 左右車輪間距離 [mm] */

void  Odometry_Init(void);
void  Odometry_ResetPosition(void); /* x,y,θだけリセット（距離は維持） */
void  Odometry_Update(int32_t left_count, int32_t right_count);
float Odometry_GetX(void);        /* x座標 [mm] */
float Odometry_GetY(void);        /* y座標 [mm] */
float Odometry_GetTheta(void);    /* 向き [rad] */
float Odometry_GetDistance(void); /* 総走行距離 [mm] */

#ifdef __cplusplus
}
#endif

#endif /* ODOMETRY_H */
