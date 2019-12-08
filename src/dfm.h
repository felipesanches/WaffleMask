class FM_Operator {
  public:
	unsigned char AM;
	unsigned char AR;
	unsigned char DR;
	unsigned char MULT;
	unsigned char RR;
	unsigned char SL;
	unsigned char TL;
	unsigned char DT2;
	unsigned char RS;
	unsigned char DT;
	unsigned char D2R;
	unsigned char SSGMODE; // (BIT 4 = 0 Disabled, 1 Enabled, BITS 0,1,2 SSG_MODE)
};

class FM_Instrument {
	unsigned char ALG;
	unsigned char FB;
	unsigned char LFO; // (FMS on YM2612, PMS on YM2151)
	unsigned char LFO2; // (AMS on YM2612, AMS on YM2151)
	FM_Operator operators[4];
};

class DFM_Module {
  public:
	unsigned char system;
	unsigned char num_instruments;
	FM_Instrument* fm_instruments;

	DFM_Module* load_dfm(const char* filename);
};
