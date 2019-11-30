/***************************************************************************

  2612intf.c

  The YM2612 emulator supports up to 2 chips.
  Each chip has the following connections:
  - Status Read / Control Write A
  - Port Read / Data Write A
  - Control Write B
  - Data Write B

***************************************************************************/

#include <stdlib.h>
#include <stddef.h>	// for NULL
#include "mamedef.h"
#include "fm.h"
#include "2612intf.h"


typedef struct _ym2612_state ym2612_state;
struct _ym2612_state
{
	void *			chip;
};


extern UINT8 CHIP_SAMPLING_MODE;
extern INT32 CHIP_SAMPLE_RATE;

#define MAX_CHIPS	0x02
static ym2612_state YM2612Data[MAX_CHIPS];
static int* GensBuf[0x02] = {NULL, NULL};
static UINT8 ChipFlags = 0x00;


/* update request from fm.c */
void ym2612_update_request(void *param)
{
	ym2612_state *info = (ym2612_state *)param;
	ym2612_update_one(info->chip, DUMMYBUF, 0);
}

/***********************************************************/
/*    YM2612                                               */
/***********************************************************/

//static STREAM_UPDATE( ym2612_stream_update )
void ym2612_stream_update(UINT8 ChipID, stream_sample_t **outputs, int samples)
{
	ym2612_state *info = &YM2612Data[ChipID];
	ym2612_update_one(info->chip, outputs, samples);
}


//static DEVICE_START( ym2612 )
int device_start_ym2612(UINT8 ChipID, int clock)
{
	//static const ym2612_interface dummy = { 0 };
	//ym2612_state *info = get_safe_token(device);
	ym2612_state *info;
	int rate;
	int chiptype;

	if (ChipID >= MAX_CHIPS)
		return 0;
	
	chiptype = clock&0x80000000;
	clock&=0x3fffffff;
	
	info = &YM2612Data[ChipID];
	rate = clock/72;
	if (! (ChipFlags & 0x04))	// if not ("double rate" required)
		rate /= 2;
	if ((CHIP_SAMPLING_MODE == 0x01 && rate < CHIP_SAMPLE_RATE) ||
		CHIP_SAMPLING_MODE == 0x02)
		rate = CHIP_SAMPLE_RATE;

	/**** initialize YM2612 ****/
	info->chip = ym2612_init(info, clock, rate, NULL, NULL);
	return rate;
}

//static DEVICE_STOP( ym2612 )
void device_stop_ym2612(UINT8 ChipID)
{
	ym2612_state *info = &YM2612Data[ChipID];
	ym2612_shutdown(info->chip);
}

//static DEVICE_RESET( ym2612 )
void device_reset_ym2612(UINT8 ChipID)
{
	ym2612_state *info = &YM2612Data[ChipID];
	ym2612_reset_chip(info->chip);
}

//READ8_DEVICE_HANDLER( ym2612_r )
UINT8 ym2612_r(UINT8 ChipID, offs_t offset)
{
	ym2612_state *info = &YM2612Data[ChipID];
	return ym2612_read(info->chip, offset & 3);
}

//WRITE8_DEVICE_HANDLER( ym2612_w )
void ym2612_w(UINT8 ChipID, offs_t offset, UINT8 data)
{
	ym2612_state *info = &YM2612Data[ChipID];
	ym2612_write(info->chip, offset & 3, data);
}

UINT8 ym2612_status_port_a_r(UINT8 ChipID, offs_t offset)
{
	return ym2612_r(ChipID, 0);
}
UINT8 ym2612_status_port_b_r(UINT8 ChipID, offs_t offset)
{
	return ym2612_r(ChipID, 2);
}
UINT8 ym2612_data_port_a_r(UINT8 ChipID, offs_t offset)
{
	return ym2612_r(ChipID, 1);
}
UINT8 ym2612_data_port_b_r(UINT8 ChipID, offs_t offset)
{
	return ym2612_r(ChipID, 3);
}

void ym2612_control_port_a_w(UINT8 ChipID, offs_t offset, UINT8 data)
{
	ym2612_w(ChipID, 0, data);
}
void ym2612_control_port_b_w(UINT8 ChipID, offs_t offset, UINT8 data)
{
	ym2612_w(ChipID, 2, data);
}
void ym2612_data_port_a_w(UINT8 ChipID, offs_t offset, UINT8 data)
{
	ym2612_w(ChipID, 1, data);
}
void ym2612_data_port_b_w(UINT8 ChipID, offs_t offset, UINT8 data)
{
	ym2612_w(ChipID, 3, data);
}


void ym2612_set_options(UINT8 Flags)
{
	ChipFlags = Flags;
	ym2612_setoptions(Flags);

	return;
}

void ym2612_set_mute_mask(UINT8 ChipID, UINT32 MuteMask)
{
	ym2612_state *info = &YM2612Data[ChipID];
	ym2612_set_mutemask(info->chip, MuteMask);
	return;
}
