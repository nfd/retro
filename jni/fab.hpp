#ifndef FABBUFFER_H
#define FABBUFFER_H

#include <inttypes.h>
#include <android_native_app_glue.h>

enum {
	FAB_FMT_ARGB8888 = 1,
	FAB_FMT_RGBA8888,
	FAB_FMT_RGB565
};

struct fabbuffer {
	// NB stride is in pixels. This is not so obvious...
	// Format is always RGBA8888, so bpp is always 4
	int width, height, stride, bpp;
	int fab_fmt;
	void *bits;

	fabbuffer() {} ;
	bool load_android_bitmap(android_app *app, const char *filename);
};

struct fab_rect {
	int x, y, w, h;
};


/* Stuff dealing with 16-bit colour (not used at the moment) */
static inline uint16_t colour16(uint8_t r, uint8_t g, uint8_t b)
{
	return ((uint16_t)(r >> 3)) << 11
		| ((uint16_t)(g >> 2)) << 5
		| (b >> 3);
}

static inline uint8_t redportion16(uint16_t colour)
{
	return ((colour >> 11) & 0x1f) << 3;
}

static inline uint8_t greenportion16(uint16_t colour)
{
	return ((colour >> 5) & 0x3f) << 2;
}

static inline uint8_t blueportion16(uint16_t colour)
{
	return (colour & 0x1f) << 3;
}

/* Stuff dealing with 32-bit colour */
static inline uint32_t colour(uint8_t a, uint8_t r, uint8_t g, uint8_t b)
{
	return ((uint32_t)a) << 24
		| ((uint32_t)r) << 16
		| ((uint32_t)g) << 8
		| b;

}

static inline uint8_t alphaportion(uint32_t colour)
{
	return colour >> 24;
}

static inline uint8_t redportion(uint32_t colour)
{
	return (colour & 0x00FF0000) >> 16;
}

static inline uint8_t greenportion(uint32_t colour)
{
	return (colour & 0x0000FF00) >> 8;
}

static inline uint8_t blueportion(uint32_t colour)
{
	return colour & 0x000000FF;
}

static inline void putpixel(fabbuffer *buffer, int x, int y, uint32_t colour)
{
	uint32_t *pixels = (uint32_t *)buffer->bits;

	int pos = (buffer->width * y) + x;
	pixels[pos] = colour;
}

static inline void blendpixel(fabbuffer *buffer, int x, int y, uint8_t colour[4])
{
	// Source: http://stackoverflow.com/a/12016968
	// "wasted at 2am"
	uint8_t *background = (uint8_t *)buffer->bits;
	background += (buffer->width * y) + x;

    uint32_t alpha = colour[0] + 1;
    uint32_t inv_alpha = 256 - colour[0];

    background[0] = 0xff;
    background[1] = (uint8_t)((alpha * colour[1] + inv_alpha * background[1]) >> 8);
    background[2] = (uint8_t)((alpha * colour[2] + inv_alpha * background[2]) >> 8);
    background[3] = (uint8_t)((alpha * colour[3] + inv_alpha * background[3]) >> 8);
}

static inline uint16_t col_32_to_16(uint32_t colour)
{
	return colour16(redportion(colour), greenportion(colour), blueportion(colour));
}

static inline uint32_t argb8888_to_rgba8888(uint32_t colour)
{
	// TODO: Endianness means we actually are dealing with ARGB to ABGR here
	// (or in some cases XBGR)
	return (colour & 0xFF) << 16
		| (colour & 0xFF00)
		| (colour & 0xFF0000) >> 16
		| (colour & 0xFF000000);
}

static inline void fab_clear(fabbuffer *buffer, uint32_t colour)
{
	uint32_t *bits = (uint32_t *)buffer->bits;

	for(int i = 0; i < (buffer->stride * buffer->height); i++)
		bits[i] = colour;
}

void fab_blit(fabbuffer *srcbuffer, fabbuffer *destbuffer);
void fab_blitrect(fabbuffer *srcbuffer, fab_rect srcrect, fabbuffer *destbuffer, fab_rect destrect);

#endif

