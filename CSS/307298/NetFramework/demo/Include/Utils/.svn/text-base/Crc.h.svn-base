//
//  "$Id$"
//
//  Copyright (c)1992-2007, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#ifndef __DAHUA_CRC_ORDER_H__
#define __DAHUA_CRC_ORDER_H__

#include <stddef.h>
#include "Infra/IntTypes.h"
#include "Defs.h"


namespace Dahua {
namespace Utils {


///	crc16 - compute the CRC-16 for the data buffer
///	\param [in] crc: previous CRC value
///	\param [in] buffer: data pointer
///	\param [in] len: number of bytes in the buffer
///
///	\return: The updated CRC value.
///
uint16_t UTILS_API crc16(uint16_t crc, uint8_t const* buffer, size_t length);

///	crc32 - compute the CRC-32 for the data buffer
///	\param [in] buffer: data pointer
///	\param [in] len: number of bytes in the buffer
///
///	\return: The updated CRC value.
///
uint32_t UTILS_API crc32(uint8_t const* buffer, size_t length);

///	crc32 - compute the CRC-32 for the data buffer
///	\param [in] crc: previous CRC value
///	\param [in] buffer: data pointer
///	\param [in] len: number of bytes in the buffer
///
///	\return: The updated CRC value.
///
uint32_t UTILS_API crc32(uint32_t crc, uint8_t const* buffer, size_t length);


} // namespace Utils
} // namespace Dahua

#endif// __DAHUA_CRC_ORDER_H__


