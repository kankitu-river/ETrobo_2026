#ifndef TRACE_LOGGER_H
#define TRACE_LOGGER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void TraceLogger_Start(void);//ログ記録の開始
void TraceLogger_Record(int32_t reflection, int16_t steering,//ログ記録関数
                        int16_t left_power, int16_t right_power);
void TraceLogger_Dump(void);//ログのダンプ関数

#ifdef __cplusplus
}
#endif

#endif /* TRACE_LOGGER_H */
