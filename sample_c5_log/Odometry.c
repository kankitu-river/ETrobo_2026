#include "Odometry.h"
#include <math.h>

#define PI 3.14159265f

static float   fg_x;
static float   fg_y;
static float   fg_theta;
static float   fg_distance;
static int32_t fg_prev_left;
static int32_t fg_prev_right;
static int     fg_initialized;

void Odometry_Init(void)
{
    fg_x           = 0.0f;
    fg_y           = 0.0f;
    fg_theta       = 0.0f;
    fg_distance    = 0.0f;
    fg_prev_left   = 0;
    fg_prev_right  = 0;
    fg_initialized = 0;
}

void Odometry_ResetPosition(void)
{
    fg_x     = 0.0f;
    fg_y     = 0.0f;
    fg_theta = 0.0f;
}

void Odometry_Update(int32_t left_count, int32_t right_count)
{
    float dl_deg, dr_deg, dl, dr, d, dtheta;

    if (!fg_initialized) {
        fg_prev_left   = left_count;
        fg_prev_right  = right_count;
        fg_initialized = 1;
        return;
    }

    dl_deg = (float)(left_count  - fg_prev_left);
    dr_deg = (float)(right_count - fg_prev_right);
    fg_prev_left  = left_count;
    fg_prev_right = right_count;

    dl = (dl_deg / 360.0f) * 2.0f * PI * WHEEL_RADIUS_MM;
    dr = (dr_deg / 360.0f) * 2.0f * PI * WHEEL_RADIUS_MM;

    d      = (dl + dr) / 2.0f;
    dtheta = (dr - dl) / WHEEL_BASE_MM;

    fg_theta    += dtheta;
    fg_x        += d * cosf(fg_theta - dtheta / 2.0f);
    fg_y        += d * sinf(fg_theta - dtheta / 2.0f);
    fg_distance += (d > 0.0f) ? d : -d;  /* 後退も加算 */
}

float Odometry_GetX(void)        { return fg_x; }
float Odometry_GetY(void)        { return fg_y; }
float Odometry_GetTheta(void)    { return fg_theta; }
float Odometry_GetDistance(void) { return fg_distance; }
