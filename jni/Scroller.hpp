#include <android_native_app_glue.h>
#include "fab.hpp"

class Scroller {
	fabbuffer glyphSheet;
	const char *headline;
	int headlineLength;
	int speed;
	int width, height;
	int scrollPosition;
	int subScrollPosition;

public:
	Scroller( android_app *app, const char *hl, int sp, int width_in, int height_in ) ;
	void blit(fabbuffer *bmp, int tick );
};

