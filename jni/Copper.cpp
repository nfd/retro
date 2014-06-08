#include "Copper.hpp"
#include <stdlib.h>
#include <math.h>
#include <android/log.h>

#define  LOG_TAG    "retrocopper"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGW(...)  __android_log_print(ANDROID_LOG_WARN,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

#define TWO_PI (3.14159 * 2)

CopperBar::CopperBar( int ofs, uint32_t mc, int barWidth_in, int sp ) {
	offset = ofs;
	speed = sp;
	barWidth = barWidth_in;

	// sin value table in `speed` steps, normalised to 0.0 -> 1.0
	sinValues = new float[ sp ];
	for( int s = 0; s < sp; s++ )
		sinValues[ s ] = ( sin( s / float( sp ) * TWO_PI ) * 0.5 ) + 0.5;

	if( barWidth < 1 ) barWidth = 1;
	barColours = new uint32_t[ barWidth ];
	barColours[0] = mc;
	for( int i = 1; i < barWidth; i++ ) {
		int fadeout = (barWidth - i) * 100 / barWidth;

		barColours[i] = colour(
				255,
				((uint32_t)redportion(mc)) * fadeout / 100,
				((uint32_t)greenportion(mc)) * fadeout / 100,
				((uint32_t)blueportion(mc)) * fadeout / 100);
	}
}

CopperBar::~CopperBar()
{
	delete[] sinValues;
	delete[] barColours;
}

void CopperBar::blit(fabbuffer* buffer, int tick ) {
	int vertCentre = int( sinValues[ ( tick + offset ) % speed ] * buffer->height );
	//int[] colourStrip = new int[ bmp.getWidth() ];
	for( int copperLine = 0; copperLine < barWidth; copperLine++ ) {
		int above = vertCentre + copperLine;
		int below = vertCentre - copperLine;

		//Arrays.fill( colourStrip, barColours[ copperLine ] );
		if( below >= 0 && below < buffer->height ) {
			for(int i = 0; i < buffer->width; i++) {
				putpixel(buffer, i, below, barColours[copperLine]);
			}
		}
		if( above >= 0 && above < buffer->height) {
			for(int i = 0; i < buffer->width; i++) {
				putpixel(buffer, i, above, barColours[copperLine]);
			}
		}
	}   
}

