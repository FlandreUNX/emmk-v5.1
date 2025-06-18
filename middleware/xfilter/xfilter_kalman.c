//
// Created by Fland on 25-6-14.
//
#include "emmk-config.h"

#include "./xfilter_kalman.h"

/**
 * @addtogroup DebugSupport
 * @note none
 */

/*@{*/

#undef TAG
#define TAG     "xfilter-kalman"

/*@}*/

/**
 * @addtogroup moving-average filter fucntions
 * @note none
 */

/*@{*/

void xfilter_kalmen_init(xfilter_Kalman_t *filter, float initial_P, float Q, float R) {
    ASSERT(filter != NULL);

    filter->x = 0;
    filter->P = initial_P;
    filter->Q = Q;
    filter->R = R;
    filter->isInit = 0;
}


float xfilter_kalmen(xfilter_Kalman_t *filter, float new_va) {
    ASSERT(filter != NULL);

    if (filter->isInit == 0) {
        filter->isInit = 1;
        filter->x = new_va;
        return filter->x;
    }

    // 预测步骤（时间更新）
    float x_pred = filter->x;          // 无控制输入时，状态预测值=前一状态
    float P_pred = filter->P + filter->Q;  // 预测误差协方差（不确定性增加）

    // 更新步骤（测量更新）
    float K = P_pred / (P_pred + filter->R);  // 卡尔曼增益（权衡预测与测量）
    filter->x = x_pred + K * (new_va - x_pred);    // 新的状态估计
    filter->P = (1 - K) * P_pred;             // 新的估计误差协方差

    return filter->x;  // 返回滤波后的值
}

/*@}*/


/**
 * @addtogroup DebugSupport
 * @note none
 */

/*@{*/

#undef TAG

/*@}*/