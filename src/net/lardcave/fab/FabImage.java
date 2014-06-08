package net.lardcave.fab;

import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;

class FabImage {
    //private AssetManager amgr;

    public static Bitmap open(AssetManager amgr, String path)
    {
        try
        {
            return BitmapFactory.decodeStream(amgr.open(path));
        }
        catch (Exception e) {
			e.printStackTrace();
			return null;
		}
    }

    public static int getWidth(Bitmap bmp) { return bmp.getWidth(); }
    public static int getHeight(Bitmap bmp) { return bmp.getHeight(); }

    public static void getPixels(Bitmap bmp, int[] pixels)
    {
        int w = bmp.getWidth();
        int h = bmp.getHeight();
        bmp.getPixels(pixels, 0, w, 0, 0, w, h);
    }

    public static void close(Bitmap bmp)
    {
        bmp.recycle();
    }
	
}

