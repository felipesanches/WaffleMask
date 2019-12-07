#include <stddef.h>    // for NULL
#include "chips/mamedef.h"
#include "chips/fm.h"
#include "chips/2612intf.h"
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/soundcard.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>

UINT32 sampleRate = 0;
UINT16 nChannels = 2;
typedef struct waveform_16bit_stereo
{
	INT16 Left;
	INT16 Right;
} WAVE_16BS;

INT32 CHIP_SAMPLE_RATE = 0x0000;
UINT8 CHIP_SAMPLING_MODE = 0;
stream_sample_t* DUMMYBUF[0x02] = {NULL, NULL};
UINT8 IsVGMInit = 0;

#define SAMPLESIZE    sizeof(WAVE_16BS)
#define BUFSIZE_MAX   0x1000    // Maximum Buffer Size in Bytes
#define BUFSIZELD   11      // Buffer Size
#define AUDIOBUFFERS  200     // Maximum Buffer Count
//  BUFFERSIZE = 1 << BUFSIZELD (1 << 11 = 2048)
//  1 Audio-Buffer = 11.6 msec
static INT32 hWaveOut;
static bool WaveOutOpen;
UINT32 BUFFERSIZE;	// Buffer Size in Bytes
UINT32 SAMPLES_PER_BUFFER;
static char BufferOut[AUDIOBUFFERS][BUFSIZE_MAX];
UINT16 AUDIOBUFFERU = AUDIOBUFFERS;		// used AudioBuffers

UINT32 BlocksSent;
UINT32 BlocksPlayed;


UINT32 FillBuffer(WAVE_16BS* Buffer, UINT32 BufferSize)
{
#if 0
	UINT32 CurSmpl;
	WAVE_32BS TempBuf;
	INT32 CurMstVol;
	UINT32 RecalcStep;
	CA_LIST* CurCLst;
	
	//memset(Buffer, 0x00, sizeof(WAVE_16BS) * BufferSize);
	
	RecalcStep = FadePlay ? SampleRate / 44100 : 0;
	CurMstVol = RecalcFadeVolume();
	
	if (Buffer == NULL)
	{
		//for (CurSmpl = 0x00; CurSmpl < BufferSize; CurSmpl ++)
		//	InterpretFile(1);
		InterpretFile(BufferSize);
		
		if (FadePlay && ! FadeStart)
		{
			FadeStart = PlayingTime;
			RecalcStep = FadePlay ? SampleRate / 100 : 0;
		}
		//if (RecalcStep && ! (CurSmpl % RecalcStep))
		if (RecalcStep)
			CurMstVol = RecalcFadeVolume();
		
		if (VGMEnd)
		{
			if (PauseSmpls <= BufferSize)
			{
				PauseSmpls = 0;
				EndPlay = true;
			}
			else
			{
				PauseSmpls -= BufferSize;
			}
		}
		
		return BufferSize;
	}
	
	CurChipList = (VGMEnd || PausePlay) ? ChipListPause : ChipListAll;
	
	for (CurSmpl = 0x00; CurSmpl < BufferSize; CurSmpl ++)
	{
		InterpretFile(1);
		
		// Sample Structures
		//	00 - SN76496
		//	01 - YM2413
		//	02 - YM2612
		//	03 - YM2151
		//	04 - SegaPCM
		//	05 - RF5C68
		//	06 - YM2203
		//	07 - YM2608
		//	08 - YM2610/YM2610B
		//	09 - YM3812
		//	0A - YM3526
		//	0B - Y8950
		//	0C - YMF262
		//	0D - YMF278B
		//	0E - YMF271
		//	0F - YMZ280B
		//	10 - RF5C164
		//	11 - PWM
		//	12 - AY8910
		//	13 - GameBoy
		//	14 - NES APU
		//	15 - MultiPCM
		//	16 - UPD7759
		//	17 - OKIM6258
		//	18 - OKIM6295
		//	19 - K051649
		//	1A - K054539
		//	1B - HuC6280
		//	1C - C140
		//	1D - K053260
		//	1E - Pokey
		//	1F - QSound
		//	20 - YMF292/SCSP
		TempBuf.Left = 0x00;
		TempBuf.Right = 0x00;
		CurCLst = CurChipList;
		while(CurCLst != NULL)
		{
			if (! CurCLst->COpts->Disabled)
			{
				ResampleChipStream(CurCLst, &TempBuf, 1);
			}
			CurCLst = CurCLst->next;
		}
		
		// ChipData << 9 [ChipVol] >> 5 << 8 [MstVol] >> 11  ->  9-5+8-11 = <<1
		TempBuf.Left = ((TempBuf.Left >> 5) * CurMstVol) >> 11;
		TempBuf.Right = ((TempBuf.Right >> 5) * CurMstVol) >> 11;
		if (SurroundSound)
			TempBuf.Right *= -1;
		Buffer[CurSmpl].Left = Limit2Short(TempBuf.Left);
		Buffer[CurSmpl].Right = Limit2Short(TempBuf.Right);
		
		if (FadePlay && ! FadeStart)
		{
			FadeStart = PlayingTime;
			RecalcStep = FadePlay ? SampleRate / 100 : 0;
		}
		if (RecalcStep && ! (CurSmpl % RecalcStep))
			CurMstVol = RecalcFadeVolume();
		
		if (VGMEnd)
		{
			if (! PauseSmpls)
			{
				//if (! FullBufFill)
				if (! EndPlay)
				{
					EndPlay = true;
					break;
				}
			}
			else //if (PauseSmpls)
			{
				PauseSmpls --;
			}
		}
	1}
	
	return CurSmpl;
#endif
#define k 6
	UINT32 CurSmpl;
	static int i=0;
	static int j=0;
	for (CurSmpl = 0x00; CurSmpl < BufferSize; CurSmpl ++)
	{
		i++;
		if (i > k) i = 0;
		j++;
		if (j > k) j = 0;
		Buffer[CurSmpl].Left = i > k/2 ? -0x8000 : 0x7FFF;
		Buffer[CurSmpl].Right = j > 1.4 * k/2 ? -0x8000 : 0x7FFF;
	}
	return BufferSize;
}


void WaveOutLinuxCallBack(void)
{
	UINT16 CurBuf;
	WAVE_16BS* TempBuf;
	UINT32 WrtSmpls;
	
	if (! WaveOutOpen){
		printf("Device not opened.\n");
		return;
	}
	
	CurBuf = BlocksSent % AUDIOBUFFERU;
	TempBuf = (WAVE_16BS*)BufferOut[CurBuf];
	
	WrtSmpls = FillBuffer(TempBuf, SAMPLES_PER_BUFFER);
	
	write(hWaveOut, TempBuf, WrtSmpls * SAMPLESIZE);
	BlocksSent ++;
	BlocksPlayed ++;
	
	return;
}

UINT8 StopStream(void)
{
	UINT32 RetVal;
	
	if (! WaveOutOpen)
		return 0xD8;	// Thread is not active
	
	WaveOutOpen = false;
	
	close(hWaveOut);
	
	return 0x00;
}

UINT8 StartStream()
{
	UINT32 RetVal;
	UINT32 ArgVal;
	
	if (WaveOutOpen){
		printf("Thread is already active!\n");
		return 0xD0;
	}

	// Init Audio
	BUFFERSIZE = 1 << BUFSIZELD;
	SAMPLES_PER_BUFFER = BUFFERSIZE / SAMPLESIZE;
	if (AUDIOBUFFERU > AUDIOBUFFERS)
		AUDIOBUFFERU = AUDIOBUFFERS;
	
	hWaveOut = open("/dev/dsp", O_WRONLY);
	if (hWaveOut < 0)
	{
		printf("waveOutOpen failed!\n");
		return 0xC0;		// waveOutOpen failed
	}
	WaveOutOpen = true;
	
	ArgVal = (AUDIOBUFFERU << 16) | BUFSIZELD;
	RetVal = ioctl(hWaveOut, SNDCTL_DSP_SETFRAGMENT, &ArgVal);
	if (RetVal)
		printf("Error setting Fragment Size!\n");
	ArgVal = AFMT_S16_NE;
	RetVal = ioctl(hWaveOut, SNDCTL_DSP_SETFMT, &ArgVal);
	if (RetVal)
		printf("Error setting Format!\n");
	ArgVal = nChannels;
	RetVal = ioctl(hWaveOut, SNDCTL_DSP_CHANNELS, &ArgVal);
	if (RetVal)
		printf("Error setting Channels!\n");
	ArgVal = sampleRate;
	RetVal = ioctl(hWaveOut, SNDCTL_DSP_SPEED, &ArgVal);
	if (RetVal)
		printf("Error setting Sample Rate!\n");
	
	return 0x00;
}


int main(){
	//device_reset_ym2612(0);
	StartStream();
	while (true){
		WaveOutLinuxCallBack();
	}
	return 0;
}
