#include "Starfield.hpp"

#include <stdlib.h>
#include "fab.hpp"

Starfield::Starfield( int sp, bool fs, int sz, uint32_t col, int mx, int my ) {
	speed = sp;
	fractionalSpeed = fs;
	numberOfStars = sz;
	wrappingX = mx;
	wrappingY = my;
	colour = col;

	verticalPositions = new int[sz];
	horizontalPositions = new int[sz];

	for( int c = 0; c < sz; c++ ) {
		verticalPositions[c] = random() % my;
		horizontalPositions[c] = random() % mx;
	}
}

Starfield::~Starfield() {
	delete[] verticalPositions;
	delete[] horizontalPositions;
}

void Starfield::updatePositions( int tick ) {
	for( int c = 0; c < numberOfStars; c++ ) {
		// positive speed, moves this far each tick
		if( fractionalSpeed == false ) horizontalPositions[ c ] -= speed;
		// fractional speed, move one pixel every N ticks
		else if( tick % speed == 0 ) horizontalPositions[ c ]--;

		if( horizontalPositions[ c ] < 0 ) {
			horizontalPositions[ c ] += wrappingX;
			verticalPositions[ c ] = random() % wrappingY;
		}
	}
}

void Starfield::blit(fabbuffer* buffer, int tick ) {
	updatePositions( tick );
	int32_t width = buffer->width;
	int32_t height = buffer->height;

	for( int c = 0; c < numberOfStars; c++ ) { 
		int adjustedX = horizontalPositions[ c ];
		int adjustedY = verticalPositions[ c ];
		if( wrappingX != width ) adjustedX = int( adjustedX / float( wrappingX ) * width );
		if( wrappingY != height ) adjustedY = int( adjustedY / float( wrappingY ) * height );
		putpixel(buffer, adjustedX, adjustedY, colour);
		// Larger stars:
		/*
		if(adjustedX < width - 1)
			putpixel(buffer, adjustedX + 1, adjustedY, colour);
		if(adjustedX > 0)
			putpixel(buffer, adjustedX - 1, adjustedY, colour);
		if(adjustedY > 0)
			putpixel(buffer, adjustedX, adjustedY-1, colour);
		if(adjustedY < height - 1)
			putpixel(buffer, adjustedX, adjustedY+1, colour);
		*/
	}
}

