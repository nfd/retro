// Major clag from android-ndk/samples/native-plasma 

#include <android_native_app_glue.h>
#include <android/log.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <jni.h>

#include "Starfield.hpp"
#include "Copper.hpp"
#include "Scroller.hpp"
#include "fab.hpp"

#define  LOG_TAG    "retro"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGW(...)  __android_log_print(ANDROID_LOG_WARN,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

const int NUM_STARFIELDS = 4;
const int NUM_COPPERS = 3;

//#define FULL_RESOLUTION_BACKGROUND_LOGO_WHICH_WZDD_DOESNT_APPROVE_OF

class Engine {
private:
	android_app *app;
	bool animating;
	uint32_t frameCount;
	Starfield *starfields[NUM_STARFIELDS];
	CopperBar *coppers[NUM_COPPERS];
	Scroller *scroller;
	fabbuffer buffer;
	fabbuffer bgpic;
	bool did_setup;
	fab_rect bgpic_dest;
	int32_t width, height;

public:
	Engine(android_app *app_in) : app(app_in) {
		animating = false;
		frameCount = 0;
		did_setup = false;

		// To make width and height a function of the display, see the commented
		// lines in setup().
		width = 120;
		height = 90;

		bgpic.load_android_bitmap(app, "background.png");
#ifndef FULL_RESOLUTION_BACKGROUND_LOGO_WHICH_WZDD_DOESNT_APPROVE_OF
		// Scale the logo to the virtual screen size.
		bgpic_dest.w = width * 80 / 100;
		bgpic_dest.h = (bgpic_dest.w * bgpic.height) / bgpic.width;
		bgpic_dest.x = (width / 2) - (bgpic_dest.w / 2);
		bgpic_dest.y = (height / 2) - (bgpic_dest.h / 2);
#endif
	}

	void setup(void)
	{
		assert(app->window != NULL);
		
		//int32_t ANativeWindow_getWidth(app->window) / MASTER_SCALE;
		//int32_t ANativeWindow_getHeight(app->window) / MASTER_SCALE;
		//assert(width > 0 && height > 0);
#ifdef FULL_RESOLUTION_BACKGROUND_LOGO_WHICH_WZDD_DOESNT_APPROVE_OF
		// Scale the logo to the device screen size.
		{
			int32_t window_width = ANativeWindow_getWidth(app->window);
			int32_t window_height = ANativeWindow_getHeight(app->window);
			bgpic_dest.w = window_width * 80 / 100;
			bgpic_dest.h = (bgpic_dest.w * bgpic.height) / bgpic.width;
			bgpic_dest.x = (window_width / 2) - (bgpic_dest.w / 2);
			bgpic_dest.y = (window_height / 2) - (bgpic_dest.h / 2);
		}
#endif

		buffer.width = width;
		buffer.stride = width; 
		buffer.height = height;
		buffer.bpp = 4;
		buffer.fab_fmt = FAB_FMT_ARGB8888;
		buffer.bits = malloc(width * height * buffer.bpp);

		starfields[0] = new Starfield( 4, true, 10, colour(255, 20, 20, 50), width, height );
		starfields[1] = new Starfield( 3, true, 10, colour(255, 50, 50, 80), width, height );
		starfields[2] = new Starfield( 1, false, 10, colour(255, 150, 150, 180), width, height );
		starfields[3] = new Starfield( 2, false, 10, colour(255, 200, 200, 220), width, height );

		// NB this is actually *half* the copper width, as coppers fade out in both
		// directions from their centre line. 
		int copper_width = height * 7 / 100;

		coppers[0] = new CopperBar( 0, colour(255, 100, 255, 100), copper_width, 100 );
		coppers[1] = new CopperBar( 10, colour(255, 255, 100, 100), copper_width, 100 );
		coppers[2] = new CopperBar( 20, colour(255, 100, 100, 255), copper_width, 100 );

		//scroller = new Scroller(app, "~ ~ byomb 2014 ~ ~ greetz to wzdd - heliotic - andor salga - kake - luna ~ ~ http://progsoc.org/~curious/software/art/retro/ ", 1, buffer.width / 8, buffer.height / 5 );
		scroller = new Scroller(app, "~ ~ BYOMB 2014 ~ ~ GREETZ TO WzDD - HELIOTIC - ANDOR SALGA - KAKE - LUNA ~ ~ HTTP://PROGSOC.ORG/~CURIOUS/SOFTWARE/ART/RETRO/ ", 1, buffer.width / 8, buffer.height / 5 );

		did_setup = true;
	}

	~Engine() {
		if (did_setup) {
			int i;

			for(i = 0; i < NUM_STARFIELDS; i++)
				delete starfields[i];

			for(i = 0; i < NUM_COPPERS; i++)
				delete coppers[i];

			delete scroller;

			free(buffer.bits);
		}
	}

	bool is_animating(void) {
		return animating;
	}

	void handle_cmd(int32_t cmd) {
		switch (cmd) {
			case APP_CMD_INIT_WINDOW:
				if (app->window != NULL) {
					// Would like to setup() here but Android 2.x gives us a 1x1 window
					// when we do
					draw();
				}
				break;
			case APP_CMD_TERM_WINDOW:
				animating = false;
				break;
			case APP_CMD_LOST_FOCUS:
				animating = false;
				draw();
				break;
		}
	}

	int32_t handle_input(AInputEvent *event) {
		if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
			animating = true;
			return 1;
		} else if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_KEY) {
			/*
			LOGI("Key event: action=%d keyCode=%d metaState=0x%x",
					AKeyEvent_getAction(event),
					AKeyEvent_getKeyCode(event),
					AKeyEvent_getMetaState(event));
					*/
		}

		return 0;
	}

	void stop(void) {
		animating = false;
	}

	void draw(void) {
		if (app->window == NULL) {
			// No window.
			return;
		}

		ANativeWindow_Buffer android_destbuffer;
		fabbuffer fab_destbuffer;
		if (ANativeWindow_lock(app->window, &android_destbuffer, NULL) < 0) {
			LOGW("Unable to lock window buffer");
			return;
		}

		// Wrap it in a fabbuffer. TODO: Niceify
		fab_destbuffer.width = android_destbuffer.width;
		fab_destbuffer.height = android_destbuffer.height;
		fab_destbuffer.stride = android_destbuffer.stride;
		fab_destbuffer.bpp = (android_destbuffer.format == WINDOW_FORMAT_RGB_565 ? 2 : 4);
		fab_destbuffer.fab_fmt = (android_destbuffer.format == WINDOW_FORMAT_RGB_565 ? FAB_FMT_RGB565 : FAB_FMT_RGBA8888);
		fab_destbuffer.bits = android_destbuffer.bits;

		//LOGI("Window buffer format %x", android_destbuffer.format);

		if(did_setup == false)
			setup();

		//fill_plasma(&buffer, time_ms);
		draw_internal(&fab_destbuffer);

		ANativeWindow_unlockAndPost(app->window);

		frameCount += 1;
	}

	void draw_internal(fabbuffer *fab_destbuffer) {
		fab_clear(&buffer, 0xFF000000);

		int i;

		starfields[0]->blit(&buffer, frameCount);
		starfields[1]->blit(&buffer, frameCount);

		for(i = 0; i < NUM_COPPERS; i++)
			coppers[i]->blit(&buffer, frameCount);

		fab_rect bgpic_src;
		bgpic_src.x = bgpic_src.y = 0;
		bgpic_src.w = bgpic.width;
		bgpic_src.h = bgpic.height;

#ifdef FULL_RESOLUTION_BACKGROUND_LOGO_WHICH_WZDD_DOESNT_APPROVE_OF
		fab_blit(&buffer, fab_destbuffer);
		fab_blitrect(&bgpic, bgpic_src, fab_destbuffer, bgpic_dest);
		fab_clear(&buffer, 0x00000000);
#else
		fab_blitrect(&bgpic, bgpic_src, &buffer, bgpic_dest);
#endif

		starfields[2]->blit(&buffer, frameCount);
		starfields[3]->blit(&buffer, frameCount);

		scroller->blit(&buffer, frameCount);

		// Write everything to the output buffer.
		fab_blit(&buffer, fab_destbuffer);
	}
};

extern "C" {

static int32_t engine_handle_input(struct android_app* app, AInputEvent* event) {
    Engine *engine = (Engine*)app->userData;
	return engine->handle_input(event);

}

static void engine_handle_cmd(struct android_app* app, int32_t cmd) {
    Engine *engine = (Engine*)app->userData;
	engine->handle_cmd(cmd);
}

void android_main(struct android_app* state) {
	static int init;

	Engine engine(state);

	// Make sure glue isn't stripped.
	app_dummy();

	state->userData = &engine;
	state->onAppCmd = engine_handle_cmd;
	state->onInputEvent = engine_handle_input;

	if (!init) {
		// TODO: This is all static setup, so not sure if anything to be done here.
		init = 1;
	}

	// loop waiting for stuff to do.

	while (1) {
		// Read all pending events.
		int ident;
		int events;
		struct android_poll_source* source;

		// If not animating, we will block forever waiting for events.
		// If animating, we loop until all events are read, then continue
		// to draw the next frame of animation.
		while ((ident=ALooper_pollAll(engine.is_animating() ? 0 : -1, NULL, &events,
						(void**)&source)) >= 0) {

			// Process this event.
			if (source != NULL) {
				source->process(state, source);
			}

			// Check if we are exiting.
			if (state->destroyRequested != 0) {
				LOGI("Engine thread destroy requested!");
				engine.stop();
				return;
			}
		}

		if (engine.is_animating()) {
			engine.draw();
		}
	}
}
}

