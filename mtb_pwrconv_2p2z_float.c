/***************************************************************************//**
* \file mtb_pwrconv_2p2z_float.c
* \version 1.0
* \brief Provides API implementation for the Power Conversion 2P2Z floating point regulator.
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

#include "mtb_pwrconv_2p2z_float.h"
#include <string.h>

cy_rslt_t mtb_pwrconv_2p2z_float_init(mtb_stc_pwrconv_reg_2p2z_float_ctx_t * ctx,
                                      mtb_stc_pwrconv_reg_2p2z_float_cfg_t const * cfg)
{
    (void)memset(&ctx->dat, 0, sizeof(ctx->dat)); /* Reset internal variables */
    ctx->cfg = *cfg; /* Initializing coefficients */
    return MTB_PWRCONV_RSLT_SUCCESS; /* For future capability */
}


/* [] END OF FILE */
