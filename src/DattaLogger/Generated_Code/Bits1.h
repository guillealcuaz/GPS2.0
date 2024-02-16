/* ###################################################################
**     This component module is generated by Processor Expert. Do not modify it.
**     Filename    : Bits1.h
**     Project     : DataLogger
**     Processor   : MK64FN1M0VLL12
**     Component   : BitsIO
**     Version     : Component 02.108, Driver 01.00, CPU db: 3.00.000
**     Repository  : Kinetis
**     Compiler    : GNU C Compiler
**     Date/Time   : 2024-02-08, 12:11, # CodeGen: 119
**     Abstract    :
**         This component "BitsIO" implements a multi-bit input/output.
**         It uses selected pins of one 1-bit to 8-bit port.
**         Note: This component is set to work in Input direction only.
**     Settings    :
**          Component name                                 : Bits1
**          Port                                           : PTC
**          BitsIO_LDD                                     : BitsIO_LDD
**          Pins                                           : 1
**            Pin0                                         : 
**              Pin                                        : CMP0_IN0/PTC6/LLWU_P10/SPI0_SOUT/PDB0_EXTRG/I2S0_RX_BCLK/FB_AD9/I2S0_MCLK
**          Direction                                      : Input
**          Initialization                                 : 
**            Init. direction                              : Input
**            Init. value                                  : 0
**          Safe mode                                      : yes
**          Optimization for                               : speed
**     Contents    :
**         GetDir - bool Bits1_GetDir(void);
**         GetVal - byte Bits1_GetVal(void);
**         GetBit - bool Bits1_GetBit(byte Bit);
**
**     Copyright : 1997 - 2015 Freescale Semiconductor, Inc. 
**     All Rights Reserved.
**     
**     Redistribution and use in source and binary forms, with or without modification,
**     are permitted provided that the following conditions are met:
**     
**     o Redistributions of source code must retain the above copyright notice, this list
**       of conditions and the following disclaimer.
**     
**     o Redistributions in binary form must reproduce the above copyright notice, this
**       list of conditions and the following disclaimer in the documentation and/or
**       other materials provided with the distribution.
**     
**     o Neither the name of Freescale Semiconductor, Inc. nor the names of its
**       contributors may be used to endorse or promote products derived from this
**       software without specific prior written permission.
**     
**     THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
**     ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
**     WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
**     DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
**     ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
**     (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
**     LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
**     ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
**     (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
**     SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**     
**     http: www.freescale.com
**     mail: support@freescale.com
** ###################################################################*/
/*!
** @file Bits1.h
** @version 01.00
** @brief
**         This component "BitsIO" implements a multi-bit input/output.
**         It uses selected pins of one 1-bit to 8-bit port.
**         Note: This component is set to work in Input direction only.
*/         
/*!
**  @addtogroup Bits1_module Bits1 module documentation
**  @{
*/         

#ifndef __Bits1_H
#define __Bits1_H

/* MODULE Bits1. */

/* Include shared modules, which are used for whole project */
#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"
/* Include inherited beans */
#include "BitsIoLdd1.h"

#include "Cpu.h"

#ifdef __cplusplus
extern "C" {
#endif 




/*
** ===================================================================
**     Method      :  Bits1_GetDir (component BitsIO)
**     Description :
**         This method returns direction of the component.
**     Parameters  : None
**     Returns     :
**         ---        - Direction of the component (always <false>, Input only)
**                      <false> = Input, <true> = Output
** ===================================================================
*/
#define Bits1_GetDir() (BitsIoLdd1_GetDir(BitsIoLdd1_DeviceData))

/*
** ===================================================================
**     Method      :  Bits1_GetVal (component BitsIO)
**     Description :
**         This method returns an input value.
**           a) direction = Input  : reads the input value from the
**                                   pins and returns it
**           b) direction = Output : returns the last written value
**         Note: This component is set to work in Input direction only.
**     Parameters  : None
**     Returns     :
**         ---        - Input value (0 to 1)
** ===================================================================
*/
#define Bits1_GetVal() ((byte)BitsIoLdd1_GetVal(BitsIoLdd1_DeviceData))

bool Bits1_GetBit(byte Bit);
/*
** ===================================================================
**     Method      :  Bits1_GetBit (component BitsIO)
**     Description :
**         This method returns the specified bit of the input value.
**           a) direction = Input  : reads the input value from pins
**                                   and returns the specified bit
**           b) direction = Output : returns the specified bit
**                                   of the last written value
**         Note: This component is set to work in Input direction only.
**     Parameters  :
**         NAME       - DESCRIPTION
**         Bit        - Number of the bit to read (0 to 0)
**     Returns     :
**         ---        - Value of the specified bit (FALSE or TRUE)
**                      FALSE = "0" or "Low", TRUE = "1" or "High"
** ===================================================================
*/

/* END Bits1. */

#ifdef __cplusplus
}  /* extern "C" */
#endif 

#endif 
/* ifndef __Bits1_H */
/*!
** @}
*/
/*
** ###################################################################
**
**     This file was created by Processor Expert 10.5 [05.21]
**     for the Freescale Kinetis series of microcontrollers.
**
** ###################################################################
*/
