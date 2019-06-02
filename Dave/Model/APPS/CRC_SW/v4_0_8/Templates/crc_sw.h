/**
 * @file crc_sw.h
 * @date 2015-05-04
 *
 * NOTE:
 * This file is generated by DAVE. Any manual modification done to this file will be lost when the code is
 * regenerated.
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
 * 2015-05-04:
 *     - Removed magic numbers
 *
 * 2017-11-30:
 *     - Fix inversion
 *
 * @endcond
 *
 */

#ifndef CRC_SW_H
#define CRC_SW_H

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************************************************************
 * HEADER FILES
 **********************************************************************************************************************/
#include <xmc_common.h>
#include <DAVE_Common.h>
#include "crc_sw_conf.h"

/**
 * @ingroup CRC_SW_publicparam
 * @{
 */
/*********************************************************************************************************************
 * MACROS
 ********************************************************************************************************************/
#define CRC_SW_ZERO       (0U)
#define CRC_SW_ONE        (1U)
#define CRC_SW_EIGHT      (8U)
#define CRC_SW_SIXTEEN    (16U)
#define CRC_SW_TWENTYFOUR (24U)
#define CRC_SW_THIRTYTWO  (32U)
/*********************************************************************************************************************
 * ENUMS
 ********************************************************************************************************************/
/**
 * @ingroup CRC_SW_enumerations
 * @{
 */
/**
 * @brief This enumeration holds the CRC initialization status.
 */
typedef enum CRC_SW_STATUS
{
  CRC_SW_STATUS_SUCCESS = 0U,	/**< APP initialization is successful */
  CRC_SW_STATUS_FAILURE = 1U	/**< APP initialization is failure */
} CRC_SW_STATUS_t;

/**
 * @}
 */

/**********************************************************************************************************************
* DATA STRUCTURES
**********************************************************************************************************************/
 /**
 * @ingroup CRC_SW_datastructures
 * @{
 */
 
/**
 * @brief This structure holds all the static configuration parameters for CRC.
 */
typedef struct CRC_SW_CONFIG
{
  uint32_t crc_width;					/*!< CRC Width */
  uint32_t crc_poly;					/*!< Polynomial */
  uint32_t crc_initval;					/*!< Initial value */
  uint32_t output_inversion;		    /*!< Output inversion */

  bool input_reflection;				/*!< Input reflection */
  bool output_reflection;				/*!< Output reflection */
} CRC_SW_CONFIG_t;

/**
 * @brief Handler structure with pointers to dynamic and static parameters.
 */
typedef struct CRC_SW
{
  CRC_SW_CONFIG_t * const config;        /*!< CRC_SW configuration pointer*/
  uint32_t crc_runningval;				 /*!< Running CRC value */
  uint32_t msb_mask;					 /*!< Flag for masking the MSB */
  uint32_t crc_mask;					 /*!< Flag for masking the CRC */
  uint32_t crc_shift;					 /*!< Flag for shifting the CRC */
  uint32_t *crctable;					 /*!< Pointer for CRC table */
  uint32_t tableoffset;					 /*!< Offset is mapped with CRC width */
} CRC_SW_t;

/**
 * @}
 */

/**
 * @ingroup CRC_SW_apidoc
 * @{
 */
/**********************************************************************************************************************
 * API PROTOTYPES
***********************************************************************************************************************/
/**
 * @brief Get CRC_SW APP version.
 * @return \a DAVE_APP_VERSION_t APP version information (major, minor and patch number)
 *
 * \par<b>Description: </b><br>
 * The function can be used to check application software compatibility with a
 * specific version of the APP.
 *
 * @code
 * #include <DAVE.h>
 *
 * int main(void)
 * {
 *    DAVE_APP_VERSION_t version;
 *    DAVE_Init();
 *    version = CRC_SW_GetAppVersion();
 *    if(version.major != 4U)
 *    {
 *    }
 *    while(1)
 *    {}
 *    return 0;
 * }
 * @endcode<BR> </p>
 */
DAVE_APP_VERSION_t CRC_SW_GetAppVersion(void);

/**
 * @brief Initializes CRC_SW handle.
 * @param handle Constant pointer to CRC structure of type @ref CRC_SW_t
 *
 * \par<b>Description:</b><br>
 * Calculates the MSB mask, CRC Mask and CRC shift values from polynomial width.
 * These values are used to generate the run time CRC table. Once these
 * parameters are set in this function, the user is not supposed to change them
 * in subsequent calls of CRC_SW APIs.
 * @return @ref CRC_SW_STATUS_t
 * <BR><P ALIGN="LEFT"><B>Example:</B>
 * Pre-requisite: Instantiate CRC_SW APP and generate code before using below
 * code snippet.
 * @code
 * #include <DAVE.h>
 *
 * int main(void)
 * {
 *   DAVE_Init();  // CRC_SW_Init(&CRC_SW_0) is called inside DAVE_Init
 *   while(1)
 *   {}
 *   return 0;
 * }
 * @endcode<BR> </p>
 */
CRC_SW_STATUS_t CRC_SW_Init(CRC_SW_t *const handle);

/**
 * @brief Calculates CRC on a block of input data using table driven algorithm
 * and returns running CRC value before reflection and inversion.
 * @param  handle Constant pointer to CRC structure of type @ref CRC_SW_t
 * @param  bufferPtr  Source of data
 * @param  length Length of bufferptr
 * @return void \n
 * <BR><P ALIGN="LEFT"><B>Example:</B>
 * Pre-requisite: Instantiate CRC_SW APP and generate code before using below
 * code snippet.
 * @code
 * #include <DAVE.h>
 * #include <string.h>
 *
 * int8_t CRC_SW_Data[] = "Lorem ipsum dolor sit amet";
 *
 * int main(void)
 * {
 *   DAVE_Init();    // CRC_SW_Init(&CRC_SW_0) is called inside DAVE_Init
 *   CRC_SW_CalculateCRC(&CRC_SW_0,CRC_SW_Data,strlen((const char*)CRC_SW_Data));
 *   while(1)
 *   {}
 *   return 0;
 * }
 * @endcode<BR> </p>
 */
void CRC_SW_CalculateCRC(CRC_SW_t *const handle, void * bufferptr, uint32_t length);

/**
 * @brief Get final CRC result by doing reflection(if selected) and inversion.
 * @param handle Constant pointer to CRC structure of type @ref CRC_SW_t
 * @return uint32_t CRC result
 * <BR><P ALIGN="LEFT"><B>Example:</B>
 * Pre-requisite: Instantiate CRC_SW APP and generate code before using below
 * code snippet.
 * @code
 * #include <DAVE.h>
 * #include <string.h>
 *
 * int8_t CRC_SW_Data[] = "Lorem ipsum dolor sit amet";
 *
 * int main(void)
 * {
 *   uint32_t CRCResult = 0;
 *   DAVE_Init(); // CRC_SW_Init(&CRC_SW_0) is called inside DAVE_Init
 *   CRC_SW_CalculateCRC(&CRC_SW_0,CRC_SW_Data,strlen((const char*)CRC_SW_Data));
 *   CRCResult = CRC_SW_GetCRCResult(&CRC_SW_0);
 *   while(1)
 *   {}
 *   return 0;
 * }
 * @endcode</p>
 */
uint32_t CRC_SW_GetCRCResult(CRC_SW_t *const handle);

/**
 * @}
 */
#include "crc_sw_extern.h"

#ifdef __cplusplus
}
#endif
#endif /* CRC_SW_H */