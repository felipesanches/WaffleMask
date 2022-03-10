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
#include <pthread.h>
#include <math.h>
#include "datareader.h"
#include "waffle_ui.h"

UINT32 sampleRate = 44100*1000;
UINT8 CHIP_SAMPLING_MODE = 0;
INT32 CHIP_SAMPLE_RATE = 44100;
UINT8 IsVGMInit = 0;
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
#define AUDIOBUFFERS  4     // Maximum Buffer Count
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


void ym2612_write_reg(UINT8 chipid, UINT8 addr, UINT8 value, UINT8 a1){
	ym2612_w(chipid, a1 << 1, addr);
	ym2612_w(chipid, a1 << 1 | 1, value);  
}


UINT16 CalcFNumber(float note)
{
	  const UINT32 clockFrq = 8000000;
	  return (144*note*(pow(2, 20))/clockFrq) / pow(2, 4-1);
}

#define octaveShift 0 //for now...

float NoteToFrequency(UINT8 note)
{
	//Elegant note/freq system by diegodorado
	//Check out his project at https://github.com/diegodorado/arduinoProjects/tree/master/ym2612
	const static float freq[12] = {
		//You can create your own note frequencies here. C4#-C5. There should be twelve entries.
		//YM3438 datasheet note set
		277.2, 293.7, 311.1, 329.6, 349.2, 370.0, 392.0, 415.3, 440.0, 466.2, 493.9, 523.3
	}; 
	const static float multiplier[] = {
		0.03125f,   0.0625f,   0.125f,   0.25f,   0.5f,   1.0f,   2.0f,   4.0f,   8.0f,   16.0f,   32.0f 
	}; 
	float f = freq[note%12];
	return f * multiplier[(note/12) + octaveShift];
}

void play(UINT8 note, UINT8 velocity){
	int offset = 0;
	int octave = 2;
	int NOTE = CalcFNumber(NoteToFrequency(note));
	int lsb = NOTE % 256;
	int msb = NOTE >> 8;
	ym2612_write_reg(0, 0xA4 + offset, (octave << 3) + msb, 0);
	ym2612_write_reg(0, 0xA0 + offset, lsb, 0);

	ym2612_write_reg(0, 0x28, 0, 0); // Turn it OFF before playing anything
	ym2612_write_reg(0, 0x28, velocity != 0 ? 0xF0 : 0, 0); //Reg 0x28, Value 0xF0, A1 LOW. Key On
}

void* MidiInputThread(void* _null_ptr){
	int fd;
	unsigned char buf[128];
	int l;

	if ((fd = open("/dev/midi1", O_RDONLY, 0)) == -1){
		perror ("open /dev/midi1");
		return NULL; //exit (-1);
	}

	#define WAITING_COMMAND 0
	#define WAITING_NOTE 1
	#define WAITING_VELOCITY 2
	UINT8 state = WAITING_COMMAND;
	UINT8 note, velocity;
	while ((l = read (fd, buf, sizeof (buf))) != -1){
		int i;
		for (i = 0; i < l; i++){
			if (state == WAITING_COMMAND && buf[i] == 0x90){
				state = WAITING_NOTE;
				continue;
			}

			if (state == WAITING_NOTE){
				note = buf[i];
				state = WAITING_VELOCITY;
				continue;
			}

			if (state == WAITING_VELOCITY){
				velocity = buf[i];
				state = WAITING_COMMAND;
				play(note, velocity);
				continue;
			}
		}
	}

	close (fd);
	return NULL;
}


UINT32 FillBuffer(WAVE_16BS* Buffer, UINT32 BufferSize)
{
	ym2612_stream_update(0, StreamBufs, BufferSize);
	for (UINT32 OutPos = 0x00; OutPos < BufferSize; OutPos ++)
	{
		Buffer[OutPos].Left = CurBufL[OutPos];
		Buffer[OutPos].Right = CurBufR[OutPos];
	}
	return BufferSize;
}

void* PlaybackThread(void* _null_ptr){
	UINT32 WrtSmpls;
	
	if (! WaveOutOpen){
		printf("Device not opened.\n");
		return NULL;
	}

	while (true){		
		WrtSmpls = FillBuffer(BufferOut, SAMPLES_PER_BUFFER);
		write(hWaveOut, BufferOut, WrtSmpls * SAMPLESIZE);
	}
	return NULL;
}

UINT8 StopStream(void){
	if (! WaveOutOpen)
		return 0xD8;	// Thread is not active
	
	WaveOutOpen = false;
	close(hWaveOut);	
	return 0x00;
}

UINT8 StartStream(){
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


void setup_instrument(DMF::Instrument& instr){
	for(int a1 = 0; a1<=1; a1++){
		for(int i=0; i<3; i++){
			for(int j=0; j<4; j++){
				ym2612_write_reg(0, 0x30 + 4*j + i, instr.fm.op[j].DT & 0x07 << 4 | instr.fm.op[j].MULT & 0x0F, a1); //DT1/Multi
				ym2612_write_reg(0, 0x40 + 4*j + i, instr.fm.op[j].TL, a1); //Total Level
				ym2612_write_reg(0, 0x50 + 4*j + i, instr.fm.op[j].RS & 0x07 << 5 | instr.fm.op[j].AR & 0x1F, a1); //RS/AR
				ym2612_write_reg(0, 0x60 + 4*j + i, instr.fm.op[j].AM & 0x01 << 7 | instr.fm.op[j].D1R & 0x7F, a1); //AM/D1R
				ym2612_write_reg(0, 0x70 + 4*j + i, instr.fm.op[j].D2R, a1); //D2R
				ym2612_write_reg(0, 0x80 + 4*j + i, instr.fm.op[j].D1R & 0x0F << 4 | instr.fm.op[j].RR & 0x0F, a1); //D1L/RR
				ym2612_write_reg(0, 0x90 + 4*j + i, instr.fm.op[j].SSGMODE, a1); //SSG EG
			}
			ym2612_write_reg(0, 0xB0 + i, instr.fm.FB & 0x1F << 3 | instr.fm.ALG & 0x07, a1); // Ch FB/Algo
		}
		ym2612_write_reg(0, 0xB4, 0xC0, a1); // Both speakers on
		ym2612_write_reg(0, 0x28, 0x00, a1); // Key off
	}
}

DMF::Song song;
int active_instr = 0;

int main(int argc, char** argv){
	if (argc != 3){
		printf("usage: %s file.dmf instrument_index\n\n", argv[0]);
		return -1;
	}

	active_instr = atoi(argv[2]); //FIXME: forbid instrument index larger than num(instruments)

	DMF::DataReader* datareader = new DMF::DataReader;
	datareader->load(argv[1], song);
	printf("System: %02X\n", song.infos.system);

	device_start_ym2612(0, 8000000);
	device_reset_ym2612(0);
	setup_instrument(song.instrument[active_instr]);
	StartStream();

	pthread_t playback_thread;
	if(pthread_create(&playback_thread, NULL, PlaybackThread, NULL)) {
		fprintf(stderr, "Error creating thread\n");
		return 1;
	}

	pthread_t midi_input_thread;
	if(pthread_create(&midi_input_thread, NULL, MidiInputThread, NULL)) {
		fprintf(stderr, "Error creating thread\n");
		return 1;
	}

	WaffleUI ui;
	ui.program_loop();

	return 0;
}
