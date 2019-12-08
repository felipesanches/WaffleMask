#include <stdio.h>
#include "dfm.h"

DFM_Module* DFM_Module::load_dfm(const char* filename){
	DFM_Module* module = new DFM_Module;
	FILE* f = fopen(filename, "rb");
	if (!f)
		return NULL;

	int skip = 0;
	skip += 16; // Format String ".DelekDefleMask."
	skip ++; // file version
        fseek(f, skip, SEEK_SET);

	fread(&module->system, sizeof(unsigned char), 1, f);
	printf("System type: %02X\n", module->system);

	fclose(f);
	return module;
}
