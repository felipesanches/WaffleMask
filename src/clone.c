#include <stddef.h>    // for NULL
#include <stdlib.h>    // for malloc
#include "chips/mamedef.h"
#include "chips/fm.h"
#include "chips/2612intf.h"
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/soundcard.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>

UINT32 sampleRate = 44100;
UINT8 CHIP_SAMPLING_MODE = 0;
INT32 CHIP_SAMPLE_RATE = 44100;
UINT8 IsVGMInit = 1;
stream_sample_t* DUMMYBUF[0x02] = {NULL, NULL};

UINT16 nChannels = 2;
typedef struct waveform_16bit_stereo
{
	INT16 Left;
	INT16 Right;
} WAVE_16BS;

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
WAVE_16BS BufferOut[BUFSIZE_MAX];

INT32* CurBufL;
INT32* CurBufR;
INT32* StreamBufs[2];

UINT32 FillBuffer(WAVE_16BS* Buffer, UINT32 BufferSize)
{

#define k 600
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

	ym2612_stream_update(0, StreamBufs, BufferSize);
	for (UINT32 OutPos = 0x00; OutPos < BufferSize; OutPos ++)
	{
		Buffer[OutPos].Left = Buffer[OutPos].Left * 0.01 + 0.9 * CurBufL[OutPos];
		Buffer[OutPos].Right = Buffer[OutPos].Right * 0.01 + 0.9 * CurBufR[OutPos];
	}
	return BufferSize;
}


void WaveOutCallBack(void)
{
	UINT32 WrtSmpls;
	
	if (! WaveOutOpen){
		printf("Device not opened.\n");
		return;
	}
		
	WrtSmpls = FillBuffer(BufferOut, SAMPLES_PER_BUFFER);
	write(hWaveOut, BufferOut, WrtSmpls * SAMPLESIZE);
	return;
}

UINT8 StopStream(void)
{
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

	StreamBufs[0x00] = (INT32*)malloc(SAMPLES_PER_BUFFER * sizeof(INT32));
	StreamBufs[0x01] = (INT32*)malloc(SAMPLES_PER_BUFFER * sizeof(INT32));
	CurBufL = StreamBufs[0x00];
	CurBufR = StreamBufs[0x01];

	hWaveOut = open("/dev/dsp", O_WRONLY);
	if (hWaveOut < 0){
		printf("waveOutOpen failed!\n");
		return 0xC0;
	}

	WaveOutOpen = true;

	
	ArgVal = (AUDIOBUFFERS << 16) | BUFSIZELD;
	RetVal = ioctl(hWaveOut, SNDCTL_DSP_SETFRAGMENT, &ArgVal);
	if (RetVal) printf("Error setting Fragment Size!\n");


	ArgVal = AFMT_S16_NE;
	RetVal = ioctl(hWaveOut, SNDCTL_DSP_SETFMT, &ArgVal);
	if (RetVal) printf("Error setting Format!\n");


	ArgVal = nChannels;
	RetVal = ioctl(hWaveOut, SNDCTL_DSP_CHANNELS, &ArgVal);
	if (RetVal) printf("Error setting Channels!\n");


	ArgVal = sampleRate;
	RetVal = ioctl(hWaveOut, SNDCTL_DSP_SPEED, &ArgVal);
	if (RetVal) printf("Error setting Sample Rate!\n");
	
	return 0x00;
}


void ym2612_write_reg(UINT8 chipid, UINT8 addr, UINT8 value, UINT8 a1){
          ym2612_w(chipid, a1 << 1, addr);
          ym2612_w(chipid, a1 << 1 | 1, value);  
} 


void setup_instrument(){
 	for(int a1 = 0; a1<=1; a1++){
		for(int i=0; i<3; i++){
			//Operator 1
			ym2612_write_reg(0, 0x30 + i, 0x71, a1); //DT1/Mul
			ym2612_write_reg(0, 0x40 + i, 0x23, a1); //Total Level
			ym2612_write_reg(0, 0x50 + i, 0x5F, a1); //RS/AR
			ym2612_write_reg(0, 0x60 + i, 0x05, a1); //AM/D1R
			ym2612_write_reg(0, 0x70 + i, 0x02, a1); //D2R
			ym2612_write_reg(0, 0x80 + i, 0x11, a1); //D1L/RR
			ym2612_write_reg(0, 0x90 + i, 0x00, a1); //SSG EG

			//Operator 2
			ym2612_write_reg(0, 0x34 + i, 0x0D, a1); //DT1/Mul
			ym2612_write_reg(0, 0x44 + i, 0x2D, a1); //Total Level
			ym2612_write_reg(0, 0x54 + i, 0x99, a1); //RS/AR
			ym2612_write_reg(0, 0x64 + i, 0x05, a1); //AM/D1R
			ym2612_write_reg(0, 0x74 + i, 0x02, a1); //D2R
			ym2612_write_reg(0, 0x84 + i, 0x11, a1); //D1L/RR
			ym2612_write_reg(0, 0x94 + i, 0x00, a1); //SSG EG

			//Operator 3
			ym2612_write_reg(0, 0x38 + i, 0x33, a1); //DT1/Mul
			ym2612_write_reg(0, 0x48 + i, 0x26, a1); //Total Level
			ym2612_write_reg(0, 0x58 + i, 0x5F, a1); //RS/AR
			ym2612_write_reg(0, 0x68 + i, 0x05, a1); //AM/D1R
			ym2612_write_reg(0, 0x78 + i, 0x02, a1); //D2R
			ym2612_write_reg(0, 0x88 + i, 0x11, a1); //D1L/RR
			ym2612_write_reg(0, 0x98 + i, 0x00, a1); //SSG EG
			   
			//Operator 4
			ym2612_write_reg(0, 0x3C + i, 0x01, a1); //DT1/Mul
			ym2612_write_reg(0, 0x4C + i, 0x00, a1); //Total Level
			ym2612_write_reg(0, 0x5C + i, 0x94, a1); //RS/AR
			ym2612_write_reg(0, 0x6C + i, 0x07, a1); //AM/D1R
			ym2612_write_reg(0, 0x7C + i, 0x02, a1); //D2R
			ym2612_write_reg(0, 0x8C + i, 0xA6, a1); //D1L/RR
			ym2612_write_reg(0, 0x9C + i, 0x00, a1); //SSG EG

			ym2612_write_reg(0, 0xB0 + i, 0x32, a1); // Ch FB/Algo
			ym2612_write_reg(0, 0xB4 + i, 0xC0, a1); // Both Spks on
			ym2612_write_reg(0, 0xA4 + i, 0x22, a1); // Set Freq MSB
			ym2612_write_reg(0, 0xA0 + i, 0x69, a1); // Freq LSB
		}
		ym2612_write_reg(0, 0xB4, 0xC0, a1); // Both speakers on
		ym2612_write_reg(0, 0x28, 0x00, a1); // Key off
	}
}

//uint16_t CalcFNumber(float note)
//{
//  const uint32_t clockFrq = 8000000;
//  return (144*note*(pow(2, 20))/clockFrq) / pow(2, 4-1);
//}

void play(){
  int offset = 0;
  int octave = 2;
  int NOTE_A = 1038;//CalcFNumber();
  int lsb = NOTE_A % 256; //fNumberNotes[key] % 256;
  int msb = NOTE_A >> 8; //fNumberNotes[key] >> 8; 
  ym2612_write_reg(0, 0xA4 + offset, (octave << 3) + msb, 0);
  ym2612_write_reg(0, 0xA0 + offset, lsb, 0);
//  ym2612_w(0, 0x28, 0xF0 + offset + (setA1 << 2));

ym2612_write_reg(0, 0x28, 0xF0, 0); //Reg 0x28, Value 0xF0, A1 LOW. Key On
//ym2612_write_reg(0, 0x28, 0x00, 0); //Reg 0x28, Value 0xF0, A1 LOW. Key Off
}

int main(){
	device_start_ym2612(0, 8000000);
	device_reset_ym2612(0);

	setup_instrument();
	play();

	StartStream();
	while (true){
		WaveOutCallBack();
	}
	return 0;
}
