//
// Created by Fland on 25-6-14.
//

#ifndef XFILTYER_KALMAN_H
#define XFILTYER_KALMAN_H

#include <stdint.h>

/**
 * @addtogroup Typedef
 * @note none
 */

/*@{*/

typedef struct {
    float x;    // 当前状态估计值（待滤波的变量）
    float P;    // 估计误差协方差（不确定性）
    float Q;    // 过程噪声协方差（系统模型误差）
    float R;    // 测量噪声协方差（传感器噪声）
    uint8_t isInit;
} xfilter_Kalman_t;

/*@}*/

/**
 * @addtogroup moving-average filter fucntions
 * @note none
 */

/*@{*/

extern void xfilter_kalmen_init(xfilter_Kalman_t *filter, float initial_P, float Q, float R);

extern float xfilter_kalmen(xfilter_Kalman_t *filter, float new_va);

/*@}*/

#endif //XFILTYER_KALMAN_H
