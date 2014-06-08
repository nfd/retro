#include <assert.h>
#include <math.h>

#include "Scroller.hpp"

static const int characterSize = 8;
static const int halfCharacterSize = characterSize / 2;
static const float sinAmplitude = 0.10;

static const float TWO_PI = M_PI * 2;

/* Scrolltext characteristics */
static inline bool char_idx(char c, int *x, int *y)
{
	if(c < 32 || c > 126)
		return false;

	int idx = c - 32;
	*y = (idx / 8) * characterSize;
	*x = (idx % 8) * characterSize;
	return true;
}

Scroller::Scroller( android_app *app, const char *hl, int sp, int width_in, int height_in ) : headline(hl), speed(sp), width(width_in), height(height_in) { 
	headlineLength = strlen(headline);
	scrollPosition = 0;
	subScrollPosition = 0;

	glyphSheet.load_android_bitmap(app, "sonic.png");

	assert(glyphSheet.width == (characterSize * 8) && glyphSheet.height == characterSize * 12);
}

void Scroller::blit(fabbuffer *bmp, int tick ) {
	int charWide = (bmp->width / characterSize) + 1;
	fab_rect textrect_src, textrect_dest;

	textrect_src.w = 1;
	textrect_src.h = characterSize;

	textrect_dest.w = 1;
	textrect_dest.h = height;

	//fab_blit_32to32(&glyphSheet, bmp);
	//fab_blitrect_32to32(&glyphSheet, 0, 0, 32, 32, bmp, 0, 0, 32, 32);
	subScrollPosition += speed;
	if( subScrollPosition > characterSize ) {
		scrollPosition++;
		subScrollPosition = subScrollPosition % characterSize;
	}

	scrollPosition = scrollPosition % headlineLength;

	for( int c = 0; c < charWide; c++ ) {
		char fragmentCharacter = headline[( scrollPosition + c ) % headlineLength];
		int xPositionCharacter = c * characterSize - subScrollPosition;
		int yPositionCentre = ( bmp->height / 2 ) - halfCharacterSize;

		int charsheetX, charsheetY;
		bool result = char_idx(fragmentCharacter, &charsheetX, &charsheetY);
		assert(result == true);

		for( int subSlice = 0; subSlice < characterSize; subSlice++ ) {
			int xPosition = xPositionCharacter + subSlice;
			int yPosition = yPositionCentre;

            if( xPosition < 0 || xPosition >= bmp->width )
				continue;

			float xKey = float( xPosition ) / bmp->width;
			yPosition -= ((sin( xKey * TWO_PI ) * sinAmplitude) * bmp->height);

			textrect_src.x = charsheetX + subSlice;
			textrect_src.y = charsheetY;
			textrect_dest.x = xPosition;
			textrect_dest.y = yPosition;

			fab_blitrect(&glyphSheet, textrect_src, bmp, textrect_dest);
		}

	}
}

