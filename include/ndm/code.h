#ifndef __NDM_CODE_H__
#define __NDM_CODE_H__

#include <stdint.h>
#include <inttypes.h>

typedef uint32_t ndm_code_t;

/* ndm_code_t is unsigned integer type always */

#define NDM_CODE_PRIo	PRIo32
#define NDM_CODE_PRIu	PRIu32
#define NDM_CODE_PRIx	PRIx32
#define NDM_CODE_PRIX	PRIX32

#define NDM_CODE_SCNo	SCNo32
#define NDM_CODE_SCNu	SCNu32
#define NDM_CODE_SCNx	SCNx32

#define NDM_CODE_I(group, local)	(0x00000000 | (group << 16) | (local))
#define NDM_CODE_W(group, local)	(0x40000000 | (group << 16) | (local))
#define NDM_CODE_E(group, local)	(0x80000000 | (group << 16) | (local))
#define NDM_CODE_C(group, local)	(0xc0000000 | (group << 16) | (local))

#define NDM_FAIL_CODEGROUP			0xffd

#define NDM_OOM_CODEGROUP			0xffe

#define NDM_SPECIAL_CODEGROUP		0xfff

#define NDM_SUCCEEDED(code)			(((code) & 0x80000000) == 0x00000000)
#define NDM_FAILED(code)			(((code) & 0x80000000) == 0x80000000)
#define NDM_WARNING(code)			(((code) & 0xc0000000) == 0x40000000)
#define NDM_CODEID(code)			( (code) & 0x0fffffff)
#define NDM_CODEGROUP(code)			(((code) & 0x0fff0000) >> 16)

#endif	/* __NDM_CODE_H__ */
