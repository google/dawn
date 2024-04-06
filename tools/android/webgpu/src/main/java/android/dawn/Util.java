package android.dawn;

import android.view.Surface;

public class Util {
  static {
    System.loadLibrary("webgpu_c_bundled");
  }

  public static native long windowFromSurface(Surface surface);
}
