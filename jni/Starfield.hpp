#include <inttypes.h>
#include <android_native_app_glue.h>
#include "fab.hpp"

class Starfield {
private:
	int *verticalPositions;
	int *horizontalPositions;
	int numberOfStars;
	int speed;
	bool fractionalSpeed;
	int wrappingX;
	int wrappingY;
	uint32_t colour;

	void updatePositions( int tick );

public:
	Starfield( int sp, bool fs, int sz, uint32_t col, int mx, int my );
	~Starfield();

	void blit(fabbuffer* buffer, int tick );
};

