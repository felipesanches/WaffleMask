#include <stdio.h>
#include <zlib.h>
#include "dmf.h"

DMF_Module* DMF_Module::load_dmf(const char* filename){
	DMF_Module* module = new DMF_Module;
	gzFile f = gzopen(filename, "rb");
	if (!f)
		return NULL;

	int skip = 0;
	// Format String ".DelekDefleMask."
	gzread(f, &module->format_string, 16);
	module->format_string[16] = 0;
	printf("Format string: '%s'\n", module->format_string);

        gzseek(f, 1, SEEK_CUR); // file version

	gzread(f, &module->system, sizeof(unsigned char));
	printf("System type: %02X\n", module->system);

	gzclose(f);
	return module;
}
