//  Copyright (c)1992-2013, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day     SVN-Author         Modification
//

#ifndef __INCLUDE_DAHUA_EFS_ERASURE_H__
#define __INCLUDE_DAHUA_EFS_ERASURE_H__

#include "Infra/IntTypes.h"

namespace Dahua {
namespace EFS {

enum ECAlgorithm
{
	nullAlgorithm = 0,
	reedsolvanAlgorithm,
	maxAlgorithm
};

enum ECStatus
{
	nullStatus = 0,
	validStatus ,
	wantedStatus,
	invalidStatus,
	maxStatus
};

typedef struct tagPieceInfo_t
{
	char *buff;
	uint32_t len;
	ECStatus status;

    tagPieceInfo_t():buff(0),len(0),status(nullStatus){}
}PieceInfo;

class CErasureCode
{
public:
	   CErasureCode();
	   ~CErasureCode();
	   static void setup();
	   bool init( uint16_t n, uint16_t m, uint32_t pieceSize,  ECAlgorithm algorithm);
	   bool encode(PieceInfo *data, PieceInfo *parity);
	   bool decode(PieceInfo *data, PieceInfo *parity);
	   bool optimizeRecoverPieces(uint64_t  invalidPieces, uint64_t  validPieces,
	   							  uint64_t  excludePieces,  uint64_t  *optimizePieces );
private:
    class Internal;
    class Internal* m_internal;

};

} /* namespace EFS */
} /* namespace Dahua */
#endif /* __INCLUDE_DAHUA_EFS_ERASURE_H__ */
