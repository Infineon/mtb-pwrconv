# Copyright 2023-2024 Cypress Semiconductor Corporation
# SPDX-License-Identifier: Apache-2.0
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


#Calculate poles and zeros for voltage mode
proc get_wZero2 {wCrossoverFreq PhaseMargin wResonantFreq qualityFactor phaseErosion} {
    set wCrossoverFreq [expr {double($wCrossoverFreq)}]
    set PhaseMargin    [expr {double($PhaseMargin)}]
    set wResonantFreq  [expr {double($wResonantFreq)}]
    set qualityFactor  [expr {double($qualityFactor)}]
    set phaseErosion   [expr {double($phaseErosion)}]

    set value_Zero2 [expr {$wCrossoverFreq / tan(-1.0 * $::PI / 2.0 + $PhaseMargin * $::PI / 180.0 -
                atan2($wCrossoverFreq / $wResonantFreq, 1) +
                atan2($wCrossoverFreq / ($::SwitchingFreq * $::PI), 1) +
                atan2($wCrossoverFreq / ($qualityFactor * $wResonantFreq), (1.0 - pow($wCrossoverFreq / $wResonantFreq, 2))) -
                $phaseErosion * $::PI / 180.0)}]
    return $value_Zero2
}
proc get_phaseErosion {tsamp wCrossoverFreq} {
    set tsamp          [expr {double($tsamp)}]
    set wCrossoverFreq [expr {double($wCrossoverFreq)}]

    if {$tsamp == 0.0} {set phaseErosionCalc 0.0
    } else {set phaseErosionCalc [expr {180.0 / $::PI * atan2(-1.0 * $wCrossoverFreq * $tsamp / $::SwitchingFreq, 
                                                               1.0 - (pow($wCrossoverFreq, 2) / (pow($::SwitchingFreq, 2) * 4.0 * pow($tsamp, 2))))}]}
    return $phaseErosionCalc
}
proc get_poles_zeros_voltage_mode {l0_inductance c0_capacitance c0_esr lesr} {
    set wResonantFreq [expr {1.0 / sqrt($l0_inductance * $c0_capacitance * ($::RoMaxLoad + $c0_esr) /
                        ($::RoMaxLoad + $lesr))}]

    set resonantFreq [get_value_from_anglular $wResonantFreq]

    set wCrossoverFreq [expr {$::CrossoverFreq * 2.0 * $::PI}]

    set qualityFactor [expr {(1.0 / $wResonantFreq) * ($::RoMaxLoad + $lesr) / ($l0_inductance +
                        $c0_capacitance * ($c0_esr * $::RoMaxLoad + $::RoMaxLoad * $lesr + $lesr * $c0_esr))}]

    set wPassiveComponentsZeros [expr {1.0 / ($c0_capacitance * $c0_esr)}]

    set tsamp [expr {$::TimeDelay}]

    set phaseErosion [get_phaseErosion $tsamp $wCrossoverFreq]

    set amplitudeCrossoverFreq [expr {20.0 * log10(sqrt(1.0 + pow($wCrossoverFreq / $wPassiveComponentsZeros, 2))) - 
                                      20.0 * log10(sqrt(pow(1.0 - pow($wCrossoverFreq / $wResonantFreq, 2), 2) + 
                                                      pow($wCrossoverFreq/($qualityFactor*$wResonantFreq), 2))) +
                                      20.0 * log10($::InputNominalVoltage * ($::RoMaxLoad / ($lesr + $::RoMaxLoad)))}]

    set wPole1 [expr {1.0 / ($c0_capacitance * $c0_esr)}]

    set wPole2 [expr {$::SwitchingFreq * $::PI}]

    set wZero1 [expr {0.7 * $wResonantFreq}]

    set wZero2Helper [get_wZero2 $wCrossoverFreq $::PhaseMargin $wResonantFreq $qualityFactor $phaseErosion]

    if {$wZero2Helper < 0.0} {set wZero2 [expr {$wZero1}]}\
    else {set wZero2 [expr {$wZero2Helper}]}

    set aConst [expr {pow($::CrossoverFreq, 4) +
                      pow($::CrossoverFreq, 2) * pow($wZero1 / (2.0 * $::PI), 2) +
                      pow($::CrossoverFreq, 2) * pow($wZero2 / (2.0 * $::PI), 2) +
                      pow($wZero1 / (2.0 * $::PI), 2) * pow($wZero2 / (2.0 * $::PI), 2)}]

    set cConst [expr {pow($::CrossoverFreq, 4) +
                      pow($::CrossoverFreq, 2) * pow($wPole1 / (2.0 * $::PI), 2) +
                      pow($::CrossoverFreq, 2) * pow($wPole2 / (2.0 * $::PI), 2) +
                      pow($wPole1 / (2.0 * $::PI), 2) * pow($wPole2 / (2.0 * $::PI), 2)}]

    set wPole0 [expr {sqrt($cConst / $aConst) * pow(10, -$amplitudeCrossoverFreq / 20.0) * $::CrossoverFreq * 2.0
                * $::PI * $wZero1 * $wZero2 / ($wPole1 * $wPole2)}]

# Calculate max valid Phase Margin

    set minPhaseMargin 30.0
    set maxPhaseMargin 90.0
    set validMaxPhaseMargin -1.0
    set candidatePhaseMargin 0.0
    set tolerance 1.0

    while {$maxPhaseMargin - $minPhaseMargin > $tolerance} {
        set pmMid [expr {($maxPhaseMargin + $minPhaseMargin) / 2.0}]

    set last_wZero2 [expr {$wCrossoverFreq / tan(-1.0 * $::PI / 2.0 + $pmMid * $::PI / 180.0 -
                     atan2($wCrossoverFreq / $wResonantFreq, 1) +
                     atan2($wCrossoverFreq / ($::SwitchingFreq * $::PI), 1) +
                     atan2($wCrossoverFreq / ($qualityFactor * $wResonantFreq), (1.0 - pow($wCrossoverFreq / $wResonantFreq, 2))) -
                     $phaseErosion * $::PI / 180.0)}]

        if {$last_wZero2 > 0.0} {
            set minPhaseMargin [expr {$pmMid}]
            set candidatePhaseMargin [expr {$pmMid}]
        } else {set maxPhaseMargin [expr {$pmMid}]}
    }

# check candidate value
    if {$candidatePhaseMargin >= $minPhaseMargin} {
        set candidatePhaseMargin [expr {int($candidatePhaseMargin)}]
        set last_wZero2_Check [get_wZero2 $wCrossoverFreq $candidatePhaseMargin $wResonantFreq $qualityFactor $phaseErosion]
    # one step lower
        set pmCheckLo [expr {int($candidatePhaseMargin - $tolerance)}]
        set last_wZero2_CheckLo [get_wZero2 $wCrossoverFreq $pmCheckLo $wResonantFreq $qualityFactor $phaseErosion]
    # check one step higher
        set pmCheckHi [expr {int($candidatePhaseMargin + $tolerance)}]
        set last_wZero2_CheckHi [get_wZero2 $wCrossoverFreq $pmCheckHi $wResonantFreq $qualityFactor $phaseErosion]

        if {$last_wZero2_CheckHi > 0.0} {
            set validMaxPhaseMargin $pmCheckHi
        } elseif {$last_wZero2_Check > 0.0} {
        set validMaxPhaseMargin $candidatePhaseMargin
        } elseif {$last_wZero2_CheckLo > 0.0} {
            set validMaxPhaseMargin $pmCheckLo}
    }

# Calculate max valid Crossover Frequency

    set maxwCrosFreq [expr {2.0 * $::PI * $::SwitchingFreq / 10.0}]
    set minwCrosFreq [expr {$maxwCrosFreq / 10.0}]
    set validMaxCrossoverFreq -1.0
    set candidateCrosFreq 0.0
    set toleranceVc [get_angular_from_value 100.0]

    while {$maxwCrosFreq - $minwCrosFreq > $toleranceVc} {
        set crosFreqMid [expr {($maxwCrosFreq + $minwCrosFreq) / 2.0}]

        if {$tsamp == 0.0} {set phaseErosionIter 0.0}\
        else {set phaseErosionIter [expr {180.0 / $::PI * atan2(-1.0 * $crosFreqMid * $tsamp / $::SwitchingFreq, 1 - (pow($crosFreqMid, 2) / (pow($::SwitchingFreq, 2) * 4.0 * pow($tsamp, 2))))}]}

        set lastPos_wZero2 [expr {$crosFreqMid / tan(-1.0 * $::PI / 2.0 + $::PhaseMargin * $::PI / 180.0 -
                             atan2($crosFreqMid / $wResonantFreq, 1) +
                             atan2($crosFreqMid / ($::SwitchingFreq * $::PI), 1) +
                             atan2($crosFreqMid / ($qualityFactor * $wResonantFreq), (1.0 - pow($crosFreqMid / $wResonantFreq, 2))) -
                             $phaseErosionIter * $::PI / 180.0)}]

        if {$lastPos_wZero2 > 0.0} {
            set minwCrosFreq [expr {$crosFreqMid}]
            set candidateCrosFreq [expr {$crosFreqMid}]
        } else {set maxwCrosFreq [expr {$crosFreqMid}]}
    }

# check candidate value
    if {$candidateCrosFreq >= $minwCrosFreq} {
        set candidateCrosFreq [expr {floor($candidateCrosFreq)}]
        set phaseErosionCandidat [get_phaseErosion $tsamp $candidateCrosFreq]
        set lastPos_wZero2_Check [get_wZero2 $candidateCrosFreq $::PhaseMargin $wResonantFreq $qualityFactor $phaseErosionCandidat]
    # one step lower
        set crosFreqCheckLo [expr {int($candidateCrosFreq - $toleranceVc)}]
        set phaseErosionLo [get_phaseErosion $tsamp $crosFreqCheckLo]
        set lastPos_wZero2_Check [get_wZero2 $crosFreqCheckLo $::PhaseMargin $wResonantFreq $qualityFactor $phaseErosionLo]
    # check one step higher
        set crosFreqCheckHi [expr {int($candidateCrosFreq + $toleranceVc)}]
        set phaseErosionHi [get_phaseErosion $tsamp $crosFreqCheckHi]
        set lastPos_wZero2_CheckHi [get_wZero2 $crosFreqCheckHi $::PhaseMargin $wResonantFreq $qualityFactor $phaseErosionHi]
    
        if {$lastPos_wZero2_CheckHi > 0.0} {
            set validMaxCrossoverFreq $crosFreqCheckHi
        } elseif {$lastPos_wZero2_Check > 0.0} {
            set validMaxCrossoverFreq $candidateCrosFreq
        } elseif {$lastPos_wZero2_CheckLo > 0.0} {
            set validMaxCrossoverFreq $crosFreqCheckLo}
    }

    if {$validMaxCrossoverFreq > 0} {
    set validMaxCrossoverFreq [get_value_from_anglular $validMaxCrossoverFreq]
    set validMaxCrossoverFreq [get_round_down_value $validMaxCrossoverFreq]}

    return [dict create wPole0 $wPole0 wPole1 $wPole1 wPole2 $wPole2 wZero1 $wZero1 wZero2 $wZero2 validMaxPhaseMargin $validMaxPhaseMargin validMaxCrossoverFreq $validMaxCrossoverFreq]
}

proc get_wZero1_Pccm {wcr fpwm phErosionSimple wp1 qualityFactor wp0 PhaseMargin} {
    set wcr             [expr {double($wcr)}]
    set fpwm            [expr {double($fpwm)}]
    set phErosionSimple [expr {double($phErosionSimple)}]
    set wp1             [expr {double($wp1)}]
    set qualityFactor   [expr {double($qualityFactor)}]
    set wp0             [expr {double($wp0)}]
    set PhaseMargin     [expr {double($PhaseMargin)}]

    set value_wZero1_Pccm [expr { $wcr / tan(atan2($wcr / (2.0 * $fpwm), 1.0) -
                                  $phErosionSimple * $::PI / 180.0 +
                                  atan2($wcr / $wp1, 1.0) +
                                  atan2(($wcr / ($qualityFactor * $wp0)), (1.0 - pow($wcr / $wp0, 2))) -
                                  $::PI / 2.0 +
                                  $PhaseMargin * $::PI / 180.0)}]
    return $value_wZero1_Pccm
}

proc get_phErosionSimple {CrossoverFreq tsamp} {
    set CrossoverFreq [expr {double($CrossoverFreq)}]
    set tsamp         [expr {double($tsamp)}]

    set phErosionSimpleCalc [expr { -1.0 * 360.0 * $CrossoverFreq / $::SamplFreq * $tsamp}]
    return $phErosionSimpleCalc
}

#Calculate poles and zeros for current mode
proc get_poles_zeros_current_mode {l0_inductance c0_capacitance c0_esr lesr} {

    set fpwm [expr {$::SwitchingFreq}]

    set tpwm [expr {1.0 / $fpwm}]

    set wcr [expr {2.0 * $::PI * $::CrossoverFreq}]

    set rl [expr {$::OutputVoltage / $::OutputNominalCurrent}]

    set ri [expr {$::CurSenseGain}]

    set d_dutyCycle [expr {$::OutputVoltage / $::InputNominalVoltage}]

    set dDash [expr {1.0 - $d_dutyCycle}]

    set mc [expr {$::CompensRamp / 1000.0}]

    set wp1 [expr {(1.0 / ($rl * $c0_capacitance)) + ((1.0 / ($fpwm * $l0_inductance * $c0_capacitance)) * ($mc * $dDash - 0.5))}]

    set wp0 [expr {$::PI / $tpwm}]

    set wz1 [expr {1.0 / ($c0_esr * $c0_capacitance)}]

    set qualityFactor [expr {1.0 / ($::PI * ($mc * $dDash - 0.5))}]

    set tsamp [expr {$::TimeDelay}]

    set phErosionSimple [get_phErosionSimple $::CrossoverFreq $tsamp]

    set hAmpl [expr {20 * log10(($rl / $ri) * (1.0 / (1.0 + ($rl * $tpwm / $l0_inductance * ($mc * $dDash - 0.5))))) +
                     20 * log10(sqrt(1.0 + (pow($wcr, 2) / pow($wz1, 2)))) -
                     20 * log10(sqrt(1.0 + (pow($wcr, 2) / pow($wp1, 2)))) -
                     20 * log10(sqrt(pow(1.0 - (pow($wcr, 2) / pow($wp0, 2)), 2)) +
                     ((pow($wcr, 2)) / (pow($wp0, 2) * pow($qualityFactor, 2))))}]

    set wZero1 [get_wZero1_Pccm $wcr $fpwm $phErosionSimple $wp1 $qualityFactor $wp0 $::PhaseMargin]

    set wPole1 [expr {1.0 / ($c0_esr * $c0_capacitance)}]

    set wPole0 [expr {($wcr * sqrt(1.0 + ((pow($wcr, 2)) / (pow($wPole1, 2))))) /
                      (pow(10, ($hAmpl / 20.0)) * sqrt(1.0 + ((pow($wcr, 2)) / (pow($wZero1, 2)))))}]

    set wPole2 0

    set wZero2 0

# Calculate max valid Phase Margin
    set minPhaseMargin 30.0
    set maxPhaseMargin 90.0
    set validMaxPhaseMargin -1.0
    set candidatePhaseMargin 0.0
    set tolerance 1.0

    while {$maxPhaseMargin - $minPhaseMargin > $tolerance} {
        set pmMid [expr {($maxPhaseMargin + $minPhaseMargin) / 2.0}]

        set last_wZero1 [expr { $wcr / tan(atan2($wcr / (2.0 * $fpwm), 1.0) -
                                           $phErosionSimple * $::PI / 180.0 +
                                           atan2($wcr / $wp1, 1.0) +
                                           atan2(($wcr / ($qualityFactor * $wp0)), (1.0 - pow($wcr / $wp0, 2))) -
                                           $::PI / 2.0 +
                                           $pmMid * $::PI / 180.0)}]

        if {$last_wZero1 > 0.0} {
            set minPhaseMargin [expr {$pmMid}]
            set candidatePhaseMargin [expr {$pmMid}]
        } else {set maxPhaseMargin [expr {$pmMid}]}
    }

# check candidate value
    if {$candidatePhaseMargin >= $minPhaseMargin} {
        set candidatePhaseMargin [expr {int($candidatePhaseMargin)}]
        set last_wZero1_Check [get_wZero1_Pccm $wcr $fpwm $phErosionSimple $wp1 $qualityFactor $wp0 $candidatePhaseMargin]
    # one step lower
        set pmCheckLo [expr {int($candidatePhaseMargin - $tolerance)}]
        set last_wZero1_CheckLo [get_wZero1_Pccm $wcr $fpwm $phErosionSimple $wp1 $qualityFactor $wp0 $pmCheckLo]
    # check one step higher
        set pmCheckHi [expr {int($candidatePhaseMargin + $tolerance)}]
        set last_wZero1_CheckHi [get_wZero1_Pccm $wcr $fpwm $phErosionSimple $wp1 $qualityFactor $wp0 $pmCheckHi]
    
        if {$last_wZero1_CheckHi > 0.0} {
            set validMaxPhaseMargin $pmCheckHi
        } elseif {$last_wZero1_Check > 0.0} {
        set validMaxPhaseMargin $candidatePhaseMargin
        } elseif {$last_wZero1_CheckLo > 0.0} {
            set validMaxPhaseMargin $pmCheckLo}
    }

# Calculate max valid Crossover Frequency

    set maxwCrosFreq [expr {2.0 * $::PI * $fpwm / 10.0}]
    set minwCrosFreq [expr {$maxwCrosFreq / 10.0}]
    set validMaxCrossoverFreq -1.0
    set candidateCrosFreq 0.0
    set tolerance [get_angular_from_value 100.0]

    while {$maxwCrosFreq - $minwCrosFreq > $tolerance} {
        set crosFreqMid [expr {($maxwCrosFreq + $minwCrosFreq) / 2.0}]

        set phErosionSimpleIter [expr {-1.0 * 360.0 * $crosFreqMid /(2.0 * $::PI * $::SamplFreq) * $tsamp}]

        set lastPos_wZero1 [expr {$crosFreqMid / tan(atan2($crosFreqMid / (2.0 * $fpwm), 1.0) -
                                                  $phErosionSimpleIter * $::PI / 180.0 +
                                                  atan2($crosFreqMid / $wp1, 1.0) +
                                                  atan2(($crosFreqMid / ($qualityFactor * $wp0)), (1.0 - pow($crosFreqMid / $wp0, 2))) -
                                                  $::PI / 2.0 +
                                                  $::PhaseMargin * $::PI / 180.0)}]

        if {$lastPos_wZero1 > 0.0} {
            set minwCrosFreq [expr {$crosFreqMid}]
            set candidateCrosFreq [expr {$crosFreqMid}]
        } else {set maxwCrosFreq [expr {$crosFreqMid}]}
    }

# check candidate value
    if {$candidateCrosFreq >= $minwCrosFreq} {
        set candidateCrosFreq [expr {int($candidateCrosFreq)}]
        set srStep            [expr {int($tolerance)}]
        set maxwCrosFreqR     [expr {int($candidateCrosFreq + 5.0 * $tolerance)}]
        set minwCrosFreqR     [expr {int($candidateCrosFreq - 5.0 * $tolerance)}]

        for {set wcrIter $maxwCrosFreqR} {$wcrIter >= $minwCrosFreqR} {incr wcrIter -$srStep} \
        {
            set phErosionSimpleIter [expr { -1.0 * 360.0 * $wcrIter /(2.0 * $::PI * $::SamplFreq) * $tsamp}]
    
            set lastPos_wZero1 [expr { $wcrIter / tan(atan2($wcrIter / (2.0 * $fpwm), 1.0) -
                                                      $phErosionSimpleIter * $::PI / 180.0 +
                                                      atan2($wcrIter / $wp1, 1.0) +
                                                      atan2(($wcrIter / ($qualityFactor * $wp0)), (1.0 - pow($wcrIter / $wp0, 2))) -
                                                      $::PI / 2.0 +
                                                      $::PhaseMargin * $::PI / 180.0)}]
            if {$lastPos_wZero1 > 0} {
                set validMaxCrossoverFreq [get_value_from_anglular $wcrIter]
                break}
        }

        set validMaxCrossoverFreq [get_round_down_value $validMaxCrossoverFreq]
    }

    return [dict create wPole0 $wPole0 wPole1 $wPole1 wPole2 $wPole2 wZero1 $wZero1 wZero2 $wZero2 validMaxPhaseMargin $validMaxPhaseMargin validMaxCrossoverFreq $validMaxCrossoverFreq]
}

# From https://wiki.tcl-lang.org/page/constants
proc const {name value} {
    uplevel 1 [list set $name $value]
    uplevel 1 [list trace var $name w {error constant} ]
}


# Name of returned parameters
const RESULT "result"
const RESULT_WPOLE0 "wPole0"
const RESULT_WPOLE1 "wPole1"
const RESULT_WPOLE2 "wPole2"
const RESULT_WZERO1 "wZero1"
const RESULT_WZERO2 "wZero2"
const RESULT_VALIDPM "validMaxPhaseMargin"
const RESULT_VALIDWCR "validMaxCrossoverFreq"

# Send data to personality
proc output_results {result} {
    puts $::channelName "param:$::RESULT=$result"

    puts $::channelName "param:$::RESULT_WPOLE0=[dict get $result wPole0]"
    puts $::channelName "param:$::RESULT_WPOLE1=[dict get $result wPole1]"
    puts $::channelName "param:$::RESULT_WPOLE2=[dict get $result wPole2]"
    puts $::channelName "param:$::RESULT_WZERO1=[dict get $result wZero1]"
    puts $::channelName "param:$::RESULT_WZERO2=[dict get $result wZero2]"
    puts $::channelName "param:$::RESULT_VALIDPM=[dict get $result validMaxPhaseMargin]"
    puts $::channelName "param:$::RESULT_VALIDWCR=[dict get $result validMaxCrossoverFreq]"
}

# The number of input parameters
const NUM_OF_ARG 16

# Input parameters
set ControlMode ""
set InputNominalVoltage 0
set OutputVoltage 0
set OutputNominalCurrent 0
set TimeDelay 0
set SwitchingFreq 0
set CrossoverFreq 0
set RoMaxLoad 0
set PhaseMargin 0
set CurSenseGain 0
set CompensRamp 0
set SamplFreq 0
set l0_inductance 0
set c0_capacitance 0
set c0_esr 0
set lesr 0

# PI number
const PI 3.14159265358979


# Main function
proc main {} {
    if {$::argc != $::NUM_OF_ARG} {
        error "Script requires two input parameters"
        return 0
    }

    # Read input parameters
    set ::ControlMode [lindex $::argv 0]
    set ::InputNominalVoltage [lindex $::argv 1]
    set ::OutputVoltage [lindex $::argv 2]
    set ::OutputNominalCurrent [lindex $::argv 3]
    set ::TimeDelay [lindex $::argv 4]
    set ::SwitchingFreq [lindex $::argv 5]
    set ::CrossoverFreq [lindex $::argv 6]
    set ::RoMaxLoad [lindex $::argv 7]
    set ::PhaseMargin [lindex $::argv 8]
    set ::CurSenseGain [lindex $::argv 9]
    set ::CompensRamp [lindex $::argv 10]
    set ::SamplFreq [lindex $::argv 11]

    set l0_inductance [lindex $::argv 12]
    set l0_inductance [expr $l0_inductance / 1e6]
    set c0_capacitance [lindex $::argv 13]
    set c0_capacitance [expr $c0_capacitance / 1e6]
    set c0_esr [lindex $::argv 14]
    set c0_esr [expr $c0_esr / 1e3]
    set lesr [lindex $::argv 15]
    set lesr [expr $lesr / 1e3]

    # Calculate poles and zeros
    if {$::ControlMode == "VOLTAGE"} {set poles_zeros_dict [get_poles_zeros_voltage_mode $l0_inductance $c0_capacitance $c0_esr $lesr]}\
    else {set poles_zeros_dict [get_poles_zeros_current_mode  $l0_inductance $c0_capacitance $c0_esr $lesr]
    }

    output_results $poles_zeros_dict
}

proc get_value_from_anglular {angular} {
    set value [expr {$angular / (2.0 * $::PI)}]
    return $value
}

proc get_round_down_value {value} {
    set value [expr {int($value * 0.01) * 100}]
    return $value
}

proc get_angular_from_value {value} {
    set angular [expr {$value * 2.0 * $::PI}]
    return $angular
}


# Select output methods
set channelName stdout
if {[chan names ModusToolbox] eq "ModusToolbox"} {
    set channelName ModusToolbox
}

# Start calculation
main
