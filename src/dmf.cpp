#include <stdio.h>
#include "dmf.h"

DMF_Module* DMF_Module::load_dmf(const char* filename){
	DMF_Module* module = new DMF_Module;
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
