#ifndef LOG_REPLAY_H
#define LOG_REPLAY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void     LogReplay_Init(void);
int      LogReplay_IsFinished(void);  /* 2周完了で1を返す */
int16_t  LogReplay_GetSteering(void); /* 現在のステアリング値を返して内部カウンタを進める */
uint32_t LogReplay_GetLap(void);      /* 現在の周回数を返す */

#ifdef __cplusplus
}
#endif

#endif /* LOG_REPLAY_H */
