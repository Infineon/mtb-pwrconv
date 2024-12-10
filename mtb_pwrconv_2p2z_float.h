/***************************************************************************//**
* \file mtb_pwrconv_2p2z_float.h
* \version 1.0
* \brief Provides API declarations for the Power Conversion 2P2Z floating point regulator.
********************************************************************************
* \copyright
* (c) (2024), Cypress Semiconductor Corporation (an Infineon company) or
* an affiliate of Cypress Semiconductor Corporation. All rights reserved.
********************************************************************************
* This software, including source code, documentation and related materials
* ("Software") is owned by Cypress Semiconductor Corporation or one of its
* affiliates ("Cypress") and is protected by and subject to worldwide patent
* protection (United States and foreign), United States copyright laws and
* international treaty provisions. Therefore, you may use this Software only
* as provided in the license agreement accompanying the software package from
* which you obtained this Software ("EULA").
*
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
* non-transferable license to copy, modify, and compile the Software source
* code solely for use in connection with Cypress's integrated circuit products.
* Any reproduction, modification, translation, compilation, or representation
* of this Software except as specified above is prohibited without the express
* written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
* reserves the right to make changes to the Software without notice. Cypress
* does not assume any liability arising out of the application or use of the
* Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use in any products where a malfunction or
* failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death ("High Risk Product"). By
* including Cypress's product in a High Risk Product, the manufacturer of such
* system or application assumes all risk of such use and in doing so agrees to
* indemnify Cypress against all liability.
*******************************************************************************/

/**
 * \defgroup group_pwrconv_2p2z_float 2-pole 2-zero floating point regulator
 */

#ifndef MTB_PWRCONV_2P2Z_FLOAT_H
#define MTB_PWRCONV_2P2Z_FLOAT_H

#include "mtb_pwrconv.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \addtogroup group_pwrconv_2p2z_float
 * \{
 */

/** The regulator poles and zeroes frequencies structure */
typedef struct
{
    float32_t p[2];
    float32_t z[2];
} mtb_stc_pwrconv_reg_2p2z_float_freq_t;

/** The regulator configuration structure */
typedef struct
{
    float32_t a[3];
    float32_t b[3];
    float32_t min; /* Lower output anti-windup limit */
    float32_t max; /* Upper output anti-windup limit */
} mtb_stc_pwrconv_reg_2p2z_float_cfg_t;

/** The regulator internal data structure */
typedef struct
{
    float32_t i[3]; /* Input values */
    float32_t o[3]; /* Output values */
} mtb_stc_pwrconv_reg_2p2z_float_dat_t;

/** The regulator working context data structure */
typedef struct
{
    mtb_stc_pwrconv_reg_2p2z_float_cfg_t cfg; /* The configuration parameters */
    mtb_stc_pwrconv_reg_2p2z_float_dat_t dat; /* The filter internal data */
} mtb_stc_pwrconv_reg_2p2z_float_ctx_t;

/** Initialize the 2P2Z Regulator
 *
 * @param[out] cfg       The pointer to the configuration structure.
 * @param[out] ctx       The pointer to the data structure, which holds the computation context.
 * @return               The initialization status.
 */
cy_rslt_t mtb_pwrconv_2p2z_float_init(mtb_stc_pwrconv_reg_2p2z_float_ctx_t * ctx,
                                      mtb_stc_pwrconv_reg_2p2z_float_cfg_t const * cfg);

/** Run the 2P2Z Regulator
 *
 * @param[out] ctx       The pointer to the data structure, which holds the computation context.
 * @param[in] input      The input data - typically, a difference between the current reference and ADC result.
 * @param[in] output     The pointer to the output data - typically, the pointer to the modulator value.
 * @return               The processing status.
 */
__STATIC_FORCEINLINE cy_rslt_t mtb_pwrconv_2p2z_float_process(mtb_stc_pwrconv_reg_2p2z_float_ctx_t * ctx,
                                                              int32_t input, uint32_t * output)
{
    float32_t a; /* Accumulator */

    /* Filter calculations */
    /* This code is intentionally written by two operations in line for readability and performance purposes */
    a  = ctx->cfg.b[2] * ctx->dat.i[1]; ctx->dat.i[1] = ctx->dat.i[0];
    a += ctx->cfg.b[1] * ctx->dat.i[0]; ctx->dat.i[0] = (float32_t)input;
    a += ctx->cfg.b[0] * ctx->dat.i[0];
    a += ctx->cfg.a[2] * ctx->dat.o[1]; ctx->dat.o[1] = ctx->dat.o[0];
    a += ctx->cfg.a[1] * ctx->dat.o[0];

    /* Anti-windup limitations */
    a = (a < ctx->cfg.max) ? a : ctx->cfg.max;
    a = (a > ctx->cfg.min) ? a : ctx->cfg.min;

    ctx->dat.o[0] = a;
    *output = (uint32_t)a;

    return MTB_PWRCONV_RSLT_SUCCESS; /* For future capability */
}


/** \} group_pwrconv_2p2z_float */

#ifdef __cplusplus
}
#endif

#endif /* MTB_PWRCONV_2P2Z_FLOAT_H */

/* [] END OF FILE */
