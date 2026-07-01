#ifndef TRACE_LOGGER_H
#define TRACE_LOGGER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void TraceLogger_Start(void);//���O�L�^�̊J�n
void TraceLogger_Record(int32_t reflection, int16_t steering,
                        int16_t left_power, int16_t right_power,
                        int32_t left_count, int32_t right_count);
void TraceLogger_Dump(void);//���O�̃_���v�֐�

#ifdef __cplusplus
}
#endif

#endif /* TRACE_LOGGER_H */
