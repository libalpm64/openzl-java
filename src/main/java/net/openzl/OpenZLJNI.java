package net.openzl;

final class OpenZLJNI {
    
    private static boolean initialized = false;
    private static final String LIBRARY_NAME = "openzl_jni";
    
    static synchronized void init() {
        if (initialized) {
            return;
        }
        
        try {
            System.loadLibrary(LIBRARY_NAME);
        } catch (UnsatisfiedLinkError e) {
            try {
                NativeLibraryLoader.loadLibrary(LIBRARY_NAME);
            } catch (Exception ex) {
                throw new UnsatisfiedLinkError("Failed to load OpenZL native library: " + ex.getMessage());
            }
        }
        
        nativeInit();
        initialized = true;
    }
    
    private static native void nativeInit();
    private static native void nativeShutdown();
    
    static native long createCompressor(int graphId);
    static native void destroyCompressor(long compressorPtr);
    static native long createDecompressor();
    static native void destroyDecompressor(long decompressorPtr);
    
    static native byte[] compressSerial(long compressorPtr, byte[] src, int srcOff, int srcLen);
    static native int compressSerialToBuffer(long compressorPtr, byte[] src, int srcOff, int srcLen,
                                           byte[] dest, int destOff, int maxDestLen);
    
    static native byte[] compressNumeric(long compressorPtr, byte[] data, int elementSize, int elementCount);
    static native byte[] compressNumericInts(long compressorPtr, int[] data);
    static native byte[] compressNumericLongs(long compressorPtr, long[] data);
    static native byte[] compressNumericFloats(long compressorPtr, float[] data);
    static native byte[] compressNumericDoubles(long compressorPtr, double[] data);
    
    static native byte[] decompressSerial(long decompressorPtr, byte[] src, int srcOff, int srcLen);
    static native int decompressSerialToBuffer(long decompressorPtr, byte[] src, int srcOff, int srcLen,
                                             byte[] dest, int destOff, int maxDestLen);
    
    static native byte[] decompressNumeric(long decompressorPtr, byte[] src, int elementSize, int expectedCount);
    static native int[] decompressNumericInts(long decompressorPtr, byte[] src);
    static native long[] decompressNumericLongs(long decompressorPtr, byte[] src);
    static native float[] decompressNumericFloats(long decompressorPtr, byte[] src);
    static native double[] decompressNumericDoubles(long decompressorPtr, byte[] src);
    
    static native CompressionInfo getCompressionInfo(byte[] src);
    static native int compressBound(int srcLen);
    
    private OpenZLJNI() {
    }
}