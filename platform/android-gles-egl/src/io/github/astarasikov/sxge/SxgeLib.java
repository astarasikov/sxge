package io.github.astarasikov.sxge;

public class SxgeLib {

     static {
         System.loadLibrary("sxge_jni");
     }

     public static native void init(int width, int height);
     public static native void step();
}
