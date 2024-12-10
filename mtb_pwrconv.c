/*******************************************************************************
* \file mtb_pwrconv.c
* \version 1.0
* \brief Provides API implementation for the Power Conversion Control Middleware.
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

#include "mtb_pwrconv.h"
#include <string.h>

/* internal function prototype */
void mtb_pwrconv_run(mtb_stc_pwrconv_ctx_t * ctx);

cy_rslt_t mtb_pwrconv_init(mtb_stc_pwrconv_t const * inst)
{
    cy_rslt_t rslt = MTB_PWRCONV_RSLT_SUCCESS;

    if ((NULL == inst)            ||
        (NULL == inst->ctx)       ||
        (NULL == inst->init_hw)   ||
        (NULL == inst->enable_hw) ||
        (NULL == inst->disable_hw))
    {
        rslt = MTB_PWRCONV_RSLT_INVALID_PARAM;
    }
    else
    {
        (void)memset(inst->ctx, 0, sizeof(*inst->ctx));
        rslt  = mtb_pwrconv_set_target(inst, inst->targ);
        rslt |= inst->init_hw();
    }

    return rslt;
}


cy_rslt_t mtb_pwrconv_enable(mtb_stc_pwrconv_t const * inst)
{
    if (NULL != inst->init_reg)
    {
        (void)inst->init_reg();
    }

    return inst->enable_hw();
}


/* internal function */
void mtb_pwrconv_run(mtb_stc_pwrconv_ctx_t * ctx)
{
    ctx->state |= MTB_PWRCONV_STATE_RUN;

    if (ctx->ref != ctx->targ)
    {
        ctx->state |= MTB_PWRCONV_STATE_RAMP;
    }
}


cy_rslt_t mtb_pwrconv_start(mtb_stc_pwrconv_t const * inst)
{
    cy_rslt_t rslt = (cy_rslt_t)Cy_TrigMux_SwTrigger(inst->syncStartTrig, CY_TRIGGER_TWO_CYCLES);

    if (MTB_PWRCONV_RSLT_SUCCESS == rslt)
    {
        mtb_pwrconv_run(inst->ctx);
    }

    return rslt;
}


cy_rslt_t mtb_pwrconv_disable(mtb_stc_pwrconv_t const * inst)
{
    inst->ctx->ref    = 0U;
    inst->ctx->state &= ~(MTB_PWRCONV_STATE_RUN | MTB_PWRCONV_STATE_RAMP);

    return inst->disable_hw();
}


/* [] END OF FILE */
