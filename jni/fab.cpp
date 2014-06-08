// It's magic!
//
#include <inttypes.h>
#include <android/log.h>
#include <unistd.h>
#include <assert.h>

#include <android_native_app_glue.h>
#include <jni.h>

#include "fab.hpp"

#define  LOG_TAG    "retrofab"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGW(...)  __android_log_print(ANDROID_LOG_WARN,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

static struct {
	jclass klazz;
	jmethodID open;
	jmethodID getWidth;
	jmethodID getHeight;
	jmethodID getPixels;
	jmethodID close;
} gFabImage;

static jobject goAssetManager = NULL;
static const char *FABIMAGE_SIG = "net/lardcave/fab/FabImage";

bool fabbuffer::load_android_bitmap(android_app *app, const char *filename)
{
	// Instance of android.app.NativeActivity
	bool result = true;
	jobject oNativeActivity = app->activity->clazz; // shouldn't be 'klazz'; see native_activity.h
	jstring sFilename = NULL;
	JNIEnv *env;

	if(app->activity->vm->AttachCurrentThread(&env, NULL) != 0) {
		LOGE("AttachCurrentThread");
		goto fail;
	}

	sFilename = env->NewStringUTF(filename);

	if(goAssetManager == NULL) {
		jmethodID mGetAssets = env->GetMethodID(env->GetObjectClass(oNativeActivity), "getAssets", "()Landroid/content/res/AssetManager;");
		if(mGetAssets == NULL) {
			LOGE("mGetAssets");
			goto fail;
		}

		goAssetManager = env->CallObjectMethod(oNativeActivity, mGetAssets);
		if(goAssetManager == NULL) {
			LOGE("goAssetManager");
			goto fail;
		}
	}

	if(gFabImage.klazz == NULL) {
		// The class loader we get from env doesn't have the NativeActivity package classes. Work around it
		// using https://code.google.com/p/android/issues/detail?id=28352
		jmethodID mGetClassLoader = env->GetMethodID(env->GetObjectClass(oNativeActivity), "getClassLoader", "()Ljava/lang/ClassLoader;");
		if(mGetClassLoader == NULL) {
			LOGE("locate getClassLoader");
			goto fail;
		}

		jobject oClassLoader = env->CallObjectMethod(oNativeActivity, mGetClassLoader);
		if(oClassLoader == NULL) {
			LOGE("invoke getClassLoader");
			goto fail;
		}

		jmethodID mFindClass = env->GetMethodID(env->GetObjectClass(oClassLoader), "findClass", "(Ljava/lang/String;)Ljava/lang/Class;");
		if(mFindClass == NULL) {
			LOGE("locate findClass");
			goto fail;
		}

		jstring sFabimageSig = env->NewStringUTF(FABIMAGE_SIG);
		gFabImage.klazz = (jclass) env->CallObjectMethod(oClassLoader, mFindClass, sFabimageSig);
		if(gFabImage.klazz == NULL) {
			LOGE("Can't load FabImage");
			goto fail;
		}
		env->DeleteLocalRef(sFabimageSig);

		gFabImage.open = env->GetStaticMethodID(gFabImage.klazz, "open",
				"(Landroid/content/res/AssetManager;Ljava/lang/String;)Landroid/graphics/Bitmap;");
		if(gFabImage.open == NULL) {
			LOGE("gFabImage.open");
			goto fail;
		}

		gFabImage.getWidth = env->GetStaticMethodID(gFabImage.klazz, "getWidth", "(Landroid/graphics/Bitmap;)I");
		if(gFabImage.getWidth == NULL) {
			LOGE("gFabImage.getWidth");
			goto fail;
		}

		gFabImage.getHeight = env->GetStaticMethodID(gFabImage.klazz, "getHeight", "(Landroid/graphics/Bitmap;)I");
		if(gFabImage.getHeight == NULL) {
			LOGE("gFabImage.getHeight");
			goto fail;
		}

		gFabImage.getPixels = env->GetStaticMethodID(gFabImage.klazz, "getPixels", "(Landroid/graphics/Bitmap;[I)V");
		if(gFabImage.getPixels == NULL) {
			LOGE("gFabImage.getPixels");
			goto fail;
		}

		gFabImage.close = env->GetStaticMethodID(gFabImage.klazz, "close", "(Landroid/graphics/Bitmap;)V");
		if(gFabImage.close == NULL) {
			LOGE("gFabImage.close");
			goto fail;
		}
	}

	// After all that setup...
	jobject bitmap;

	bitmap = env->CallStaticObjectMethod(gFabImage.klazz, gFabImage.open, goAssetManager, sFilename);
	if (bitmap == NULL) {
		LOGE("invoke FabImage.open");
		goto fail;
	}

	width = stride = env->CallStaticIntMethod(gFabImage.klazz, gFabImage.getWidth, bitmap);
	height = env->CallStaticIntMethod(gFabImage.klazz, gFabImage.getHeight, bitmap);
	bpp = 4;
	fab_fmt = FAB_FMT_ARGB8888;

	LOGI("New image width %d height %d", width, height);


	{
		jintArray pixelsArray = env->NewIntArray(width * height);
		env->CallStaticVoidMethod(gFabImage.klazz, gFabImage.getPixels, bitmap, pixelsArray);

		bits = malloc(width * height * 4);

		env->GetIntArrayRegion(pixelsArray, 0, width * height, (int *)bits);
	}

	// Clean up
	env->CallStaticVoidMethod(gFabImage.klazz, gFabImage.close, bitmap);

	goto exit;

fail:
	// teehee
	result = false;
exit:
	if(sFilename)
		env->DeleteLocalRef(sFilename);
	app->activity->vm->DetachCurrentThread();
	return false;
}

// From http://www.compuphase.com/graphic/scale.htm
// 'coarse 2D scaling'

// Line blitting functions with conversion
#define BLIT_LINE(CONVERTER, SRCTYPE) \
  int NumPixels = TgtWidth; \
  int IntPart = SrcWidth / TgtWidth; \
  int FractPart = SrcWidth % TgtWidth; \
  int E = 0; \
  while (NumPixels-- > 0) { \
	SRCTYPE pixel = *Source; \
	if(pixel & 0xff000000) \
		*Target++ = CONVERTER(*Source); \
	else \
		Target++; \
    Source += IntPart; \
    E += FractPart; \
    if (E >= TgtWidth) { \
      E -= TgtWidth; \
      Source++; \
    }  \
  }

static void blit_line_32to16(uint16_t *Target, uint32_t *Source, int SrcWidth, int TgtWidth)
{
BLIT_LINE(col_32_to_16, uint32_t)
}

static void blit_line_32to32(uint32_t *Target, uint32_t *Source, int SrcWidth, int TgtWidth)
{
BLIT_LINE(uint32_t, uint32_t)
}

static void blit_line_32to32_argb_to_rgba(uint32_t *Target, uint32_t *Source, int SrcWidth, int TgtWidth)
{
BLIT_LINE(argb8888_to_rgba8888, uint32_t)
}

#define BLIT_2D(SRCTYPE, DSTTYPE, BLIT_LINE_FUNC) \
	SRCTYPE *source = (SRCTYPE *)srcbuffer->bits; \
	DSTTYPE *target = (DSTTYPE *)destbuffer->bits; \
 \
	int NumPixels = destbuffer->height; \
	int IntPart = (srcbuffer->height / destbuffer->height) * srcbuffer->width; \
	int FractPart = srcbuffer->height % destbuffer->height; \
	int E = 0; \
	SRCTYPE *PrevSource = NULL; \
 \
	while (NumPixels-- > 0) { \
		if (source == PrevSource) { \
			memcpy(target, target - destbuffer->stride, destbuffer->width * sizeof(DSTTYPE)); \
		} else { \
			BLIT_LINE_FUNC(target, source, srcbuffer->width, destbuffer->width); \
			PrevSource = source; \
		} \
		target += destbuffer->stride; \
		source += IntPart; \
		E += FractPart; \
		if (E >= destbuffer->height) { \
			E -= destbuffer->height; \
			source += srcbuffer->stride; \
		} \
	}

static void fab_blit_32to32(fabbuffer *srcbuffer, fabbuffer *destbuffer)
{
	BLIT_2D(uint32_t, uint32_t, blit_line_32to32)
}

static void fab_blit_32to32_argb_to_rgba(fabbuffer *srcbuffer, fabbuffer *destbuffer)
{
	BLIT_2D(uint32_t, uint32_t, blit_line_32to32_argb_to_rgba)
}

static void fab_blit_32to16(fabbuffer *srcbuffer, fabbuffer *destbuffer)
{
	BLIT_2D(uint32_t, uint16_t, blit_line_32to16)
}

void fab_blit(fabbuffer *srcbuffer, fabbuffer *destbuffer)
{
	assert(srcbuffer->bpp == 4);
	switch(destbuffer->bpp) {
		case 2:
			fab_blit_32to16(srcbuffer, destbuffer);
			break;
		case 4:
			if (srcbuffer->fab_fmt == destbuffer->fab_fmt)
				fab_blit_32to32(srcbuffer, destbuffer);
			else
				fab_blit_32to32_argb_to_rgba(srcbuffer, destbuffer);
			break;
	}
}


void fab_blitrect(fabbuffer *srcbuffer, fab_rect src_rect, fabbuffer *destbuffer, fab_rect dest_rect)
{
	fabbuffer src, dest;

	src.width = src_rect.w;
	src.height = src_rect.h;
	src.stride = srcbuffer->stride;
	src.bpp = srcbuffer->bpp;
	src.bits = &(((uint8_t *)srcbuffer->bits)[((srcbuffer->stride * src_rect.y ) + src_rect.x) * srcbuffer->bpp]);
	src.fab_fmt = srcbuffer->fab_fmt;

	dest.width = dest_rect.w;
	dest.height = dest_rect.h;
	dest.stride = destbuffer->stride;
	dest.bpp = destbuffer->bpp;
	dest.bits = &(((uint8_t *)destbuffer->bits)[((destbuffer->stride * dest_rect.y) + dest_rect.x) * destbuffer->bpp]);
	dest.fab_fmt = destbuffer->fab_fmt;

	fab_blit(&src, &dest);
}

