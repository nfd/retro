#include <inttypes.h>
#include <android_native_app_glue.h>

#include "fab.hpp"

class CopperBar {
	uint32_t *barColours;
	float *sinValues;
	int offset;
	int speed;
	int barWidth;

public:
	CopperBar( int ofs, uint32_t mc, int barWidth_in, int sp );
	~CopperBar();
	void blit(fabbuffer* buffer, int tick );
};

