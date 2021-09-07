#ifndef __MODBUS_LIB_H__
#define __MODBUS_LIB_H__

#define htol(x)	(((x << 8) & 0xFF00) \
		| (x >> 8))


#define CRC_MODE 0  // 0:Fast mode but worst room
					// 1:optimization room
					//

#define MB_SER_PDU_SIZE_MIN     4       /*!< Minimum size of a Modbus RTU frame. */
#define MB_SER_PDU_SIZE_MAX     256     /*!< Maximum size of a Modbus RTU frame. */
#define MB_SER_PDU_SIZE_CRC     2       /*!< Size of CRC field in PDU. */
#define MB_SER_PDU_ADDR_OFF     0       /*!< Offset of slave address in Ser-PDU. */
#define MB_SER_PDU_PDU_OFF      1       /*!< Offset of Modbus-PDU in Ser-PDU. */

#define UCHAR unsigned char

unsigned short usMBCRC16( UCHAR * pucFrame, unsigned short usLen );
#endif
