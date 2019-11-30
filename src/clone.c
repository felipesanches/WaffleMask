#include <stddef.h>    // for NULL
#include "chips/mamedef.h"
#include "chips/fm.h"
#include "chips/2612intf.h"

INT32 CHIP_SAMPLE_RATE = 0x0000;
UINT8 CHIP_SAMPLING_MODE = 0;
stream_sample_t* DUMMYBUF[0x02] = {NULL, NULL};
UINT8 IsVGMInit = 0;

int main(){
	//device_reset_ym2612(0);
	return 0;
}
