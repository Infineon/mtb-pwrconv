/***************************************************************************//**
* \file mtb_pwrconv.h
* \version 1.0
* \brief Provides API declarations for the Power Conversion Control Middleware.
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
 ********************************************************************************
 * \mainpage
 ********************************************************************************
 * The Power Conversion Middleware provides a solid solution to design power convertors easily.
 *
 * \section section_pwrconv_features Features
 * - Configurable by the Power Conversion Configurator (PCC)
 * - Synchronous Buck Converter topology support
 * - Voltage-control (VCM) and Peak-current control (PCCM) modes support
 * - Multi-instance support (up to 4 instances)
 * - Multi-phase support (up to 4 phases)
 * - Auto-generated control loop and Custom (user-defined) control loop modes support
 * - Built-in 2p2z and 3p3z regulators
 * - Basic ramp generator
 * - Pre- and Post-processing user callbacks in the Auto-generated control loop mode
 * - Separately scheduled ADC sequencer group for the slow parameters
 *
 * \section section_pwrconv_glossary Glossary
 * - PCC - Power Conversion Configurator
 * - VCM - voltage control mode, firmware regulator directly controls the PWM pulse width value.
 * - PCCM - peak-current control mode, firmware regulator (outer control loop)
 * control of the DAC slope value, a reference for the comparator (the inner HW control loop)
 *
 * \section section_pwrconv_general General Description
 * The Power Conversion Middleware consists of the device-agnostic top-level API
 * source code, the power conversion library source code, and the solution personality.
 * The solution personality:
 * - provides the integration with the Power Conversion Configurator and Device Configurator.
 * - calculates the regulator coefficients
 * - generates the power conversion regulator configuration code and device-specific HW configuration code
 * - generates the control loop ISR
 * \image html solution.png
 *
 * \section section_pwrconv_quickstart Quick Start
 * Assume the power converter instance is named in the ModusToolbox&trade;
 * Device Configurator as 'myPwrConv':
 * \image html myPwrConv.png
 *
 * Then, the simplest way to use myPwrConv is to use generated myPwrConv API:
 * \snippet simple/main.c snippet_pwrconv_simple
 *
 * \section section_pwrconv_infineon Infineon Control Loop
 * The whole control loop including the regulator is generated
 * by the personality.
 * The control loop consists of:
 * - retrieving the ADC feedback value from the ADC register into \ref mtb_stc_pwrconv_ctx_t::res
 * - the error calculation:
 * \ref mtb_stc_pwrconv_ctx_t::err = \ref mtb_stc_pwrconv_ctx_t::ref - \ref mtb_stc_pwrconv_ctx_t::res
 * - optionally, the pre-process user callback execution, which is a way to apply any user-specific pre-filtering
 * to the \ref mtb_stc_pwrconv_ctx_t::err value, or perform some protection actions
 * (e.g. check ADC channel values for ranges and stop the conversion in case of any parameter violation)
 * - performing the regulation itself
 * - optionally the post-process user callback execution, which is a way to apply any user-specific post-filtering
 * to the modulator \ref mtb_stc_pwrconv_ctx_t::mod value
 * - apply the modulator \ref mtb_stc_pwrconv_ctx_t::mod value to the modulator itself
 * (update PWM duty cycle in VCM or DAC start/stop values in PCCM)
 *
 * \subsection subsection_pwrconv_pre_post_process Infineon Control Loop Callbacks
 * To use the pre- or/and post-process callbacks, the callback itself should be
 * declared at the application level, the corresponding feature should be enabled
 * in the PCC tool Controller tab:
 * \image html preCb.png
 * and the callback name should be passed into the Device Configurator GUI:
 * \image html callback.png
 * To make these callbacks faster, they can be declared as inline functions in header file -
 * in this case the header file name should be also passed into the Device Configurator GUI, as shown above.
 *
 * And then in the myHeader.h:
 * \snippet simple/myHeader.h snippet_pwrconv_pre_process
 * \note For non-inline callbacks there is no need to update the 'Header file name'
 * parameter - it can be left empty (by default).
 *
 * \section section_pwrconv_custom Custom Control Loop
 * In the Custom Control Loop mode:
 * - there is no dedicated feedback ADC channel defined in the
 * Control Loop ADC Group - all the channels are equally 'custom'.
 * Therefore, the error \ref mtb_stc_pwrconv_ctx_t::err
 * is not being calculated in the Control Loop ISR (unlike in the Infineon Control Loop mode).
 * It is the user's task to define the controlled value(s) and to perform the feedback processing and regulation.
 * - the custom control loop callback is being called instead of
 * the default (Infineon Control Loop mode) bult-in regulator with pre-/post-process callbacks.
 * - the modulator update with the \ref mtb_stc_pwrconv_ctx_t::mod value works
 * exactly like in the Infineon Control Loop mode.
 * - the \ref myPwrConv_set_target function is transparent -
 * the reference value nominally equals to the target value,
 * without any min/max limitations nor recalculations,
 * because the target is not predefined by the solution itself -
 * it should be defined by user at the application level.
 * \image html custom.png
 * For example to use the inline custom callback, the callback name and the header file name
 * should be entered into the Device Configurator:
 * \image html custCb.png
 * And then in the myCustom.h:
 * \snippet custom/myCustom.h snippet_pwrconv_custom
 *
 * \section section_pwrconv_scheduled Scheduled ADC Group
 * To optimize the control loop timing, some ADC measurements can be performed
 * not in the control loop ADC sequencer group, but in the separate scheduled
 * ADC group (configurable in the PCC tool ADC tab):
 * \image html schedCh.png
 * which is being periodically triggered by the \ref myPwrConv_scheduled_adc_trigger function.
 * When the scheduled ADC group measurement is done - it rises interrupt which calls the
 * scheduled user callback, which name also should be passed into the Device Configurator GUI:
 * \image html schedCb.png
 * And implemented in the application code:
 * \snippet custom/main.c snippet_pwrconv_scheduled_adc_callback
 *
 * \section section_pwrconv_modulator Modulation modes
 * \subsection subsection_pwrconv_vcm VCM
 * Voltage control modulation mode - the simple PWM, where the pulse width is directly
 * controlled by the 3P3Z regulator (in the \ref section_pwrconv_infineon  mode).
 * Supports both high resolution and regular resolution TCPWM modes (configurable in the PCC tool)
 * \image html hires.png
 *
 * \subsection subsection_pwrconv_pccm PCCM
 * Peak-Current control mode is the advanced modulation method which consists of two loops:
 *  - the inner HW inductor current control loop
 *  - the outer FW 2P2Z regulator (in Infineon control loop mode) which controls the PCCM's
 *  comparator reference (DAC slope):
 * \image html PCCM.png
 *
 * \section section_pwrconv_ramp Ramp Generator
 * The PwrConv middleware provides a simple ramp generator -
 * the \ref mtb_pwrconv_ramp function which should be called by a periodical event
 * (e.g. some timer ISR/callback, the timing is important to be determined and stable):
 * \snippet simple/main.c snippet_pwrconv_simple
 * The ramp generator updates the \ref mtb_stc_pwrconv_ctx_t::ref value so that
 * it always moves towards the \ref mtb_stc_pwrconv_ctx_t::targ value with the steps defined by
 * \ref mtb_stc_pwrconv_t::rampStep based on Ramp update period and Ramp slope
 * parameters configurable in the PCC tool:
 * \image html ramp.png
 *
 * \section section_pwrconv_syncstart Synchronous Start
 * When there are multiple instances with the same switching frequencies,
 * there might be a need to start them simultaneously with specified phase shift,
 * to avoid the control loop execution overlapping.
 * In this case the 'Initial phase' parameter could be used:
 * \image html initPhase.png
 * in conjunction with a generated pwrconv_start() function (common for all the instances)
 * instead of regular instance-based [instance_name]_start() functions:
 * \snippet vcmDual/snippet.c snippet_pwrconv_syncstart
 *
 * \section section_pwrconv_multiphase Multi-Phase
 * The multiple interleaved conversion phases allows to divide a total power between multiple parallel
 * power circuits (switches, inductors) for better heat dissipation, reliability,
 * and power density of the converter.
 * Also, in terms of EMC the multi-phasing method lowers magnitude and spreads the spectrum of both
 * the electromagnetic emissions and the voltage/current ripples on power lines.
 * The PwrConv Buck topology allow up to 4 phases (in both VCM and PCCM modes) controlled by the same FW regulator
 * (configurable in the PCC tool)
 * \image html multiphase.png
 *
 * \defgroup group_pwrconv_macros Macros
 * \{
 *   \defgroup group_pwrconv_status Status
 *   \defgroup group_pwrconv_states States
 *   \defgroup group_pwrconv_types Types
 * \}
 * \defgroup group_pwrconv_data_structures Data Structures
 * \defgroup group_pwrconv_func_types Function Type Definitions
 * \defgroup group_pwrconv_functions Functions
 * \defgroup group_pwrconv_gen_func Generated Functions
 */

#ifndef MTB_PWRCONV_H
#define MTB_PWRCONV_H

#include "cy_pdl.h"

#ifdef __cplusplus
extern "C" {
#endif

/** \addtogroup group_pwrconv_status
 *  \{ */
/** Return the Power Conversion operation status of type cy_rslt_t: successful */
#define MTB_PWRCONV_RSLT_SUCCESS          CY_RSLT_SUCCESS
/** Return the Power Conversion operation status of type cy_rslt_t: invalid input parameter */
#define MTB_PWRCONV_RSLT_INVALID_PARAM    CY_RSLT_CREATE(CY_RSLT_TYPE_ERROR, CY_RSLT_MODULE_MIDDLEWARE_PWRCONV, 1UL)
/** \} group_pwrconv_status */

/** \addtogroup group_pwrconv_types
 *  \{ */
#define MTB_PWRCONV_BUCK (0U) /**< The Buck power converter */
#define MTB_PWRCONV_PFC  (1U) /**< The PFC power converter (for future capability) */
/** \} group_pwrconv_types */

/** \addtogroup group_pwrconv_states
 *  \{ */
/** State/status/event flags */
#define MTB_PWRCONV_STATE_RUN        (0x01UL)       /**< The running flag, indicates that the converter is currently
                                                     *   running
                                                     */
#define MTB_PWRCONV_STATE_RAMP       (0x02UL)       /**< The ramping flag, indicates that the converter currently
                                                     *   is changing the reference value smoothly to the specified
                                                     *   target value
                                                     */
#define MTB_PWRCONV_STATE_ALL        (0xFFFFFFFFUL) /**< The mask to select all of the power converter
                                                     *   instance states, for future capability
                                                     */
/** \} group_pwrconv_states */



/** \addtogroup group_pwrconv_func_types
 *  \{ */

/** The type for Power Conversion generated function
 *
 * This function is generated by the configurator and used in the middleware data-driven code.
 */
typedef cy_rslt_t (* mtb_func_pwrconv_t) (void);

/** \} group_pwrconv_func_types */

/**
 * \addtogroup group_pwrconv_data_structures
 * \{
 */

/** The context data structure */
typedef struct
{
    uint32_t  targ; /**< The desired target reference value, updated by the \ref mtb_pwrconv_set_target() function. */
    uint32_t   ref; /**< The current working reference value, updated by the \ref mtb_pwrconv_ramp() function. */
    uint32_t   res; /**< The measured result, the intermediate value, being copied from the ADC result register by
                     *   generated FW or by the DMA and then being used to calculate the error value and also being used
                     *   for the monitoring/protection purpose by the \ref myPwrConv_Vout_get_result() function.
                     */
    int32_t    err; /**< The intermediate error value - the difference between the reference and measured value */
    uint32_t   mod; /**< The modulator value, typically being updated by the generated FW regulator
                     *   (or by user code in case of custom control loop),
                     *   and then copied into PWM or DAC within the generated ISR,
                     *   common for all interleaved phases.
                     */
    uint32_t state; /**< The converter state/status flags \ref group_pwrconv_states,
                     *   typically accessed by \ref mtb_pwrconv_get_state() in user code.
                     */
} mtb_stc_pwrconv_ctx_t;


/** The instance structure,
 *  which includes all the instance-specific settings:
 *  type, reference, ramping, context, regulator, HW-integration, etc.
 */
typedef struct
{
    uint8_t       type; /**< The converter type/topology, \ref group_pwrconv_types */

    /* Reference */
    uint32_t      targ; /**< The initial target value */
    uint32_t   targMax; /**< The maximal acceptable target value, in millivolts */
    uint32_t   targMin; /**< The minimal acceptable target value, in millivolts */
    uint16_t    refNum; /**< The numerator for reference calculation,
                         *   equals to the channel gain (internal and external) * ADC resolution,
                         *   used by the \ref mtb_pwrconv_set_target to recalculate the target millivolts
                         *   into the reference ADC counts in the \ref section_pwrconv_infineon mode.
                         */
    uint16_t    refDen; /**< The denominator for reference calculation,
                         *   equals to the ADC reference voltage in millivolts,
                         *   used by the \ref mtb_pwrconv_set_target to recalculate the target millivolts
                         *   into the reference ADC counts in the \ref section_pwrconv_infineon mode.
                         */
    /* Ramp */
    uint16_t  rampStep; /**< The reference update ramping step, must be positive and non-zero */

    /* RAM context */
    mtb_stc_pwrconv_ctx_t * ctx; /**< The pointer to the context RAM structure with converter live data */

    /* Regulator */
    mtb_func_pwrconv_t init_reg; /**< The pointer to the topology-specific regulator initialization function */

    /* HW */
    uint32_t        syncStartTrig; /**< The synchronous starting TrigMux line */
    mtb_func_pwrconv_t    init_hw; /**< The pointer to the topology-specific HW initialization function */
    mtb_func_pwrconv_t  enable_hw; /**< The pointer to the topology-specific HW enabling function */
    mtb_func_pwrconv_t disable_hw; /**< The pointer to the topology-specific HW disabling function */
} mtb_stc_pwrconv_t;
/** \} group_pwrconv_data_structures */


/**
 * \addtogroup group_pwrconv_functions
 * \{
 */


/** Initialize the Power Conversion Control System.
 *
 * This function initializes all the HW and FW resources of the Power Conversion Solution.
 *
 * @param[in] inst The pointer to the power convertor instance structure.
 * @return         The result of the function operation \ref group_pwrconv_status
 *                 - \ref MTB_PWRCONV_RSLT_SUCCESS - all the configuration data is valid,
 *                 - \ref MTB_PWRCONV_RSLT_INVALID_PARAM - some configuration data is invalid.
 */
cy_rslt_t mtb_pwrconv_init(mtb_stc_pwrconv_t const * inst);


/** Enables the Power Conversion Control System.
 *
 * This function enables (prepares to run) the Power Conversion components: PWM, ADC and regulator.
 *
 * @param[in] inst The pointer to the power converter instance structure.
 * @return         The result of the function operation \ref group_pwrconv_status
 *                 Currently it returns always \ref MTB_PWRCONV_RSLT_SUCCESS, for future capabilities.
 */
cy_rslt_t mtb_pwrconv_enable(mtb_stc_pwrconv_t const * inst);


/** Starts the Power Conversion Control System.
 *
 * This function triggers the synchronous start of the Power Conversion PWM(s)
 *
 * @param[in] inst The pointer to the power converter instance structure.
 *
 * @return         The result of the function operation \ref group_pwrconv_status
 *                 Possible error codes from underlying Trigger Multiplexer PDL API.
 */
cy_rslt_t mtb_pwrconv_start(mtb_stc_pwrconv_t const * inst);


/** Disables (stops) the Power Conversion Control System.
 *
 * This function disables (stops) the Power Conversion components: stops PWM(s) and DAC(s) (if used),
 * and disables ADC interrupts.
 *
 * @param[in] inst The pointer to the power converter instance structure
 * @return         The result of the function operation \ref group_pwrconv_status
 *                 Currently it returns always \ref MTB_PWRCONV_RSLT_SUCCESS, for future capabilities.
 */
cy_rslt_t mtb_pwrconv_disable(mtb_stc_pwrconv_t const * inst);


/**
 * Returns the state of a Power Conversion Control System instance.
 *
 * @param[in] inst The pointer to the power converter instance structure.
 * @param[in] mask The mask to get one or more specified power converter states.
 * @return         The power converter state \ref group_pwrconv_states.
 */
__STATIC_INLINE uint32_t mtb_pwrconv_get_state(mtb_stc_pwrconv_t const * inst, uint32_t mask)
{
    return (inst->ctx->state & mask);
}


/** Sets the desired target reference value \ref mtb_stc_pwrconv_ctx_t::targ for the \ref section_pwrconv_ramp.
 * In the \ref MTB_PWRCONV_STATE_RUN state, it also triggers
 * \ref MTB_PWRCONV_STATE_RAMP, see \ref  mtb_pwrconv_get_state
 *
 * \ref mtb_stc_pwrconv_ctx_t::targ is recalculated from millivolts into feedback ADC counts using the
 * \ref mtb_stc_pwrconv_t::refNum and \ref mtb_stc_pwrconv_t::refDen values in the \ref section_pwrconv_infineon mode.
 *
 * Also, in the \ref section_pwrconv_infineon mode, the target value is limited by
 * the \ref mtb_stc_pwrconv_t::targMax and \ref mtb_stc_pwrconv_t::targMin values -
 * they are defined by the min/max controlled parameter (Vout/Iout) in the PCC tool:
 * \image html minmax.png
 *
 * In the \ref section_pwrconv_custom mode, \ref mtb_stc_pwrconv_t::refNum and \ref mtb_stc_pwrconv_t::refDen are
 * both initialized as '1', and \ref mtb_stc_pwrconv_t::targMin and \ref mtb_stc_pwrconv_t::targMax are
 * the minimal and maximal uint32 values correspondingly.
 *
 * However, all these values can be customized in the application code by storing the
 * instance configuration structure in RAM (configurable in the Device Configurator):
 * \image html cfgRam.png
 *
 * @param[in] inst The pointer to the power converter instance structure.
 * @param[in] targ The target value to be set.
 *                 The valid range is defined by
 *                 \ref mtb_stc_pwrconv_t::targMin and
 *                 \ref mtb_stc_pwrconv_t::targMax.
 * @return         The reference value validness status \ref group_pwrconv_status
 *                 - \ref MTB_PWRCONV_RSLT_SUCCESS - the reference value is valid,
 *                 - \ref MTB_PWRCONV_RSLT_INVALID_PARAM - the reference value is outside a valid range.
 *
 * \funcusage See \ref section_pwrconv_ramp section
 */
__STATIC_INLINE cy_rslt_t mtb_pwrconv_set_target(mtb_stc_pwrconv_t const * inst, uint32_t targ)
{
    cy_rslt_t rslt = MTB_PWRCONV_RSLT_SUCCESS;

    if ((inst->targMax < targ) || (inst->targMin > targ))
    {
        rslt = MTB_PWRCONV_RSLT_INVALID_PARAM;
    }
    else
    {
        inst->ctx->targ = (uint16_t)(targ * (uint32_t)inst->refNum / (uint32_t)inst->refDen);
        if (0UL != mtb_pwrconv_get_state(inst, MTB_PWRCONV_STATE_RUN))
        {
            inst->ctx->state |= MTB_PWRCONV_STATE_RAMP;
        }
    }

    return rslt;
}


/** Generates the reference ramping for soft-start and target changing features.
 *
 * See the \ref section_pwrconv_ramp section for details.
 *
 * @param[in] inst The pointer to the power converter instance structure.
 *
 * \funcusage See the \ref section_pwrconv_ramp section
 */
__STATIC_INLINE void mtb_pwrconv_ramp(mtb_stc_pwrconv_t const * inst)
{
    if ((0UL != mtb_pwrconv_get_state(inst, MTB_PWRCONV_STATE_RUN)) &&
        (inst->ctx->ref != inst->ctx->targ))
    {
        int16_t locDiff = (int16_t)inst->ctx->targ - (int16_t)inst->ctx->ref;

        /* The absolute of locDiff is less than or equal to rampStep. */
        if ((locDiff <= (int16_t)inst->rampStep) && (-locDiff <= (int16_t)inst->rampStep))
        {
            inst->ctx->ref    = inst->ctx->targ;
            inst->ctx->state &= ~MTB_PWRCONV_STATE_RAMP;
        }
        else if (locDiff < 0)
        {
            inst->ctx->ref   -= inst->rampStep;
        }
        else /* locDiff > 0 */
        {
            inst->ctx->ref   += inst->rampStep;
        }
    }
}


/** Calculates the error value from the result \ref mtb_stc_pwrconv_ctx_t::res and
 *  reference \ref mtb_stc_pwrconv_ctx_t::ref values and stores it into the
 *  context structure \ref mtb_stc_pwrconv_ctx_t::err.
 *
 * @param[in] ctx    The pointer to the power converter instance context structure.
 * @return           The error value, also saved into \ref mtb_stc_pwrconv_ctx_t::err.
 *
 * \funcusage See \ref myPwrConv_get_error()
 */
__STATIC_FORCEINLINE int32_t mtb_pwrconv_get_error(mtb_stc_pwrconv_ctx_t * ctx)
{
    ctx->err = (int32_t)ctx->ref - (int32_t)ctx->res; /* Calculate the error value */
    return ctx->err;
}


/** \} group_pwrconv_functions */

#ifdef __cplusplus
}
#endif

#endif /* MTB_PWRCONV_H */

/* [] END OF FILE */
