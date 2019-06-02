/**
 * @file crc_sw.c
 * @date 2015-05-20
 *
 * NOTE:
 * This file is generated by DAVE. Any manual modification done to this file will be lost when the code is regenerated.
 */
/**
 * @cond
 ***********************************************************************************************************************
 * CRC_SW v4.0.8 - computes CRC for a given data
 *
 * Copyright (c) 2015-2017, Infineon Technologies AG
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,are permitted provided that the
 * following conditions are met:
 *
 *   Redistributions of source code must retain the above copyright notice, this list of conditions and the  following
 *   disclaimer.
 *
 *   Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the
 *   following disclaimer in the documentation and/or other materials provided with the distribution.
 *
 *   Neither the name of the copyright holders nor the names of its contributors may be used to endorse or promote
 *   products derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE  FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY,OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT  OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * To improve the quality of the software, users are encouraged to share modifications, enhancements or bug fixes
 * with Infineon Technologies AG (dave@infineon.com).
 ***********************************************************************************************************************
 *
 * Change History
 * --------------
 *
 * 2015-02-16:
 *     - Initial version<br>
 *
 * 2015-05-20:
 *     - Optimized four APIs to two APIs(CRC_SW_lCalculateCRC_xmc4_refin,CRC_SW_lCalculateCRC_xmc1_refin) <br>
 *
 * @endcond
 *
 */

/***********************************************************************************************************************
 * HEADER FILES
 **********************************************************************************************************************/
#include "crc_sw.h"

/***********************************************************************************************************************
 * MACROS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * LOCAL DATA
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * LOCAL ROUTINES
 **********************************************************************************************************************/
static uint32_t CRC_SW_lReflect(uint32_t data, uint32_t length);

#if (UC_FAMILY == XMC4)
static void CRC_SW_lCalculateCRC_xmc4_refin(CRC_SW_t *const handle, void * bufferptr, uint32_t length);
#endif

#if (UC_FAMILY == XMC1)
static void CRC_SW_lCalculateCRC_xmc1_refin(CRC_SW_t *const handle, void * bufferptr, uint32_t length);
#endif


/**********************************************************************************************************************
* API IMPLEMENTATION
**********************************************************************************************************************/
/*
 * This function reverses all bits of data
*/
static uint32_t CRC_SW_lReflect(uint32_t data, uint32_t length)
{
  uint32_t count;
  uint32_t retval;

  retval = data & CRC_SW_ONE;
  
  for (count = CRC_SW_ONE; count < length ; count++)
  {
    data >>= CRC_SW_ONE;
    retval = (retval << CRC_SW_ONE) | (data & CRC_SW_ONE);
  }

  return retval;
}

/*
 * API to retrieve the version of the CRC_SW APP
 */
DAVE_APP_VERSION_t CRC_SW_GetAppVersion(void)
{
  DAVE_APP_VERSION_t version;

  version.major = CRC_SW_MAJOR_VERSION;
  version.minor = CRC_SW_MINOR_VERSION;
  version.patch = CRC_SW_PATCH_VERSION;

  return (version);
}

/*
 * This function initializes CRC_SW handle.
 *
*/
CRC_SW_STATUS_t CRC_SW_Init(CRC_SW_t *const handle)
{
  CRC_SW_STATUS_t status;

  if (handle != NULL)
  {
    /* calculate MSB mask, CRC Mask and shift from polynomial width */
    handle->msb_mask = (uint32_t)(CRC_SW_ONE << (handle->config->crc_width - CRC_SW_ONE));

    handle->crc_mask = (uint32_t)((handle->msb_mask - CRC_SW_ONE) | (handle->msb_mask));

    handle->crc_shift = CRC_SW_ZERO;

    handle->config->output_reflection &= handle->crc_mask;

    status = CRC_SW_STATUS_SUCCESS;
  }
  else
  {
    status = CRC_SW_STATUS_FAILURE;
  }

  return (status);
}

/*
 * This function calculates CRC on a block of data.
 */
void CRC_SW_CalculateCRC(CRC_SW_t *const handle, void * bufferptr, uint32_t length)
{
  XMC_ASSERT("CalculateCRC: NULL Handle", handle != NULL);
  XMC_ASSERT("CalculateCRC: NULL Buffer", bufferptr != NULL);

#if (UC_FAMILY == XMC4)
    CRC_SW_lCalculateCRC_xmc4_refin(handle, bufferptr, length);
#endif

#if (UC_FAMILY == XMC1)
    CRC_SW_lCalculateCRC_xmc1_refin(handle, bufferptr, length);
#endif
}

/*
 * This function calculates CRC when input reflection is enabled for xmc4.
 */
#if (UC_FAMILY == XMC4)
static void CRC_SW_lCalculateCRC_xmc4_refin(CRC_SW_t *const handle, void * bufferptr, uint32_t length)
{
  uint32_t tableindex;
  uint32_t buffer;
  uint8_t *temp;

  /* Load the initial CRC value as running value for CRC */
  handle->crc_runningval = handle->config->crc_initval;
  
  temp = (uint8_t *)bufferptr;

  while (length--)
  {
    buffer = (uint32_t) *temp;

    /* if input reflection is set */
    if (handle->config->input_reflection == true)
    {
      tableindex = ((handle->crc_runningval >> handle->crc_shift) ^ buffer) & (int32_t)0xff;

      /* compute the CRC running value from the lookup table */
      handle->crc_runningval = (*(uint32_t*)((uint32_t)handle->crctable + (tableindex * handle->tableoffset))  ^ \
                                 (handle->crc_runningval >> CRC_SW_EIGHT)) & (handle->crc_mask << handle->crc_shift);
    }
    else
    {
      /* load the lookup table value based on CRC width */
      tableindex = ((handle->crc_runningval >> ((handle->config->crc_width - CRC_SW_EIGHT) +
                     handle->crc_shift)) ^ buffer) & (uint32_t)0xff;

      /* compute the CRC running value from the lookup table */
      handle->crc_runningval = (*(uint32_t *)((uint32_t)handle->crctable + (tableindex * handle->tableoffset)) ^ \
                                 (handle->crc_runningval  << (CRC_SW_EIGHT - handle->crc_shift))) & \
                                 ((handle->crc_mask) << handle->crc_shift);
    }
    temp++;
  }
}
#endif

/*
 * This function calculates CRC when input reflection is enabled for xmc1.
 */
#if (UC_FAMILY == XMC1)
static void CRC_SW_lCalculateCRC_xmc1_refin(CRC_SW_t *const handle, void * bufferptr, uint32_t length)
{
  uint32_t crctable;
  uint32_t tableindex;
  uint32_t tableoffs;
  uint32_t data;
  uint32_t temp1;
  uint32_t shiftvalue;
  uint8_t *buffer;
  uint8_t *lookuptable;

  /* Load the initial CRC value as running value for CRC */
  handle->crc_runningval = handle->config->crc_initval;

  buffer = (uint8_t *)bufferptr;

  data = CRC_SW_ZERO;
  shiftvalue = (uint32_t)handle->crc_shift;
  crctable = (uint32_t)handle->crctable;
  tableoffs = (uint32_t)handle->tableoffset;

  while (length--)
  {
    temp1 = (uint32_t) *buffer;

    /* if input reflection is set */
    if (handle->config->input_reflection == true)
    {
      tableindex = ((handle->crc_runningval >> shiftvalue) ^ temp1) &
                    (uint32_t)0xff;
      lookuptable = (uint8_t *)(crctable + tableindex * tableoffs);
    }
    else
    {
      tableindex = ((handle->crc_runningval >>
                    ((handle->config->crc_width - CRC_SW_EIGHT) + shiftvalue)) ^ temp1) & (uint32_t)0xff;
      lookuptable = (uint8_t *)(crctable + tableindex * tableoffs);
    }
    /* load the lookup table value based on CRC width */
    if (handle->config->crc_width == CRC_SW_EIGHT)
    {
      data = *lookuptable;
    }
    else if (handle->config->crc_width == CRC_SW_SIXTEEN)
    {
      data = (*lookuptable | (*(lookuptable+1) << CRC_SW_EIGHT));
    }
    else if (handle->config->crc_width == CRC_SW_THIRTYTWO)
    {
      data = (*lookuptable | (*(lookuptable+1) << CRC_SW_EIGHT) | *(lookuptable+2) << CRC_SW_SIXTEEN |
              *(lookuptable+3) << CRC_SW_TWENTYFOUR);
    }

    /* if input reflection is set */
    if (handle->config->input_reflection == true)
    {
      handle->crc_runningval = (data ^ (handle->crc_runningval >> CRC_SW_EIGHT)) & (handle->crc_mask << shiftvalue);
    }
    else
    {
      handle->crc_runningval = (data ^ (handle->crc_runningval << (CRC_SW_EIGHT - handle->crc_shift))) & \
                               ((handle->crc_mask) << (handle->crc_shift));
    }

    buffer++;
  } /*End of "while (Length--)"*/
}
#endif

/**
 * This function returns the CRC value for the already calculated CRC by doing
 * reflection(if selected) and inversion.
 **/
uint32_t CRC_SW_GetCRCResult(CRC_SW_t *const handle)
{
  uint32_t result;
  
  if (handle != NULL)
  {
    handle->crc_runningval &= (handle->crc_mask << handle->crc_shift);
    handle->crc_runningval >>= handle->crc_shift;

    /* Do not reflect the bytes if input reflection and output reflection are set to true. Otherwise reflect the bytes*/
    if (handle->config->input_reflection != handle->config->output_reflection)
    {
      handle->crc_runningval = CRC_SW_lReflect(handle->crc_runningval, handle->config->crc_width);
    }

    handle->crc_runningval ^= handle->config->output_inversion;
    handle->crc_runningval &= handle->crc_mask;

    result = handle->crc_runningval;
  }
  else
  {
    result = CRC_SW_ZERO;
  }
  return (result);
}
