package net.openzl;

import java.nio.ByteBuffer;

public final class OpenZLDecompressor implements AutoCloseable {
    
    private final long nativePtr;
    private volatile boolean closed = false;
    
    OpenZLDecompressor() {
        this.nativePtr = OpenZLJNI.createDecompressor();
        if (this.nativePtr == 0) {
            throw new OpenZLException("Failed to create decompressor");
        }
    }
    
    public byte[] decompress(byte[] src) {
        return decompress(src, 0, src.length);
    }
    
    public byte[] decompress(byte[] src, int srcOff, int srcLen) {
        checkNotClosed();
        if (src == null) {
            throw new IllegalArgumentException("Source data cannot be null");
        }
        if (srcOff < 0 || srcLen < 0 || srcOff + srcLen > src.length) {
            throw new IndexOutOfBoundsException("Invalid offset or length");
        }
        return OpenZLJNI.decompressSerial(nativePtr, src, srcOff, srcLen);
    }
    
    public int decompress(byte[] src, int srcOff, int srcLen, byte[] dest, int destOff, int maxDestLen) {
        checkNotClosed();
        if (src == null || dest == null) {
            throw new IllegalArgumentException("Source and destination cannot be null");
        }
        checkRange(src, srcOff, srcLen);
        checkRange(dest, destOff, maxDestLen);
        return OpenZLJNI.decompressSerialToBuffer(nativePtr, src, srcOff, srcLen, dest, destOff, maxDestLen);
    }
    
    public byte[] decompress(ByteBuffer src) {
        checkNotClosed();
        if (src == null) {
            throw new IllegalArgumentException("Source buffer cannot be null");
        }
        
        if (src.hasArray()) {
            return decompress(src.array(), src.arrayOffset() + src.position(), src.remaining());
        } else {
            byte[] data = new byte[src.remaining()];
            src.get(data);
            return decompress(data);
        }
    }
    
    public int decompress(ByteBuffer src, ByteBuffer dest) {
        checkNotClosed();
        if (src == null || dest == null) {
            throw new IllegalArgumentException("Source and destination buffers cannot be null");
        }
        
        byte[] srcArray;
        int srcOff = 0;
        int srcLen = src.remaining();
        
        if (src.hasArray()) {
            srcArray = src.array();
            srcOff = src.arrayOffset() + src.position();
        } else {
            srcArray = new byte[srcLen];
            src.get(srcArray);
        }
        
        byte[] destArray;
        int destOff = 0;
        int maxDestLen = dest.remaining();
        
        if (dest.hasArray()) {
            destArray = dest.array();
            destOff = dest.arrayOffset() + dest.position();
        } else {
            destArray = new byte[maxDestLen];
        }
        
        int written = decompress(srcArray, srcOff, srcLen, destArray, destOff, maxDestLen);
        
        if (!dest.hasArray()) {
            dest.put(destArray, 0, written);
        } else {
            dest.position(dest.position() + written);
        }
        
        return written;
    }
    
    public byte[] decompressNumeric(byte[] src, int elementSize, int expectedCount) {
        checkNotClosed();
        if (src == null) {
            throw new IllegalArgumentException("Source data cannot be null");
        }
        if (elementSize <= 0 || expectedCount <= 0) {
            throw new IllegalArgumentException("Element size and expected count must be positive");
        }
        return OpenZLJNI.decompressNumeric(nativePtr, src, elementSize, expectedCount);
    }
    
    public int[] decompressNumericInts(byte[] src) {
        checkNotClosed();
        if (src == null) {
            throw new IllegalArgumentException("Source data cannot be null");
        }
        return OpenZLJNI.decompressNumericInts(nativePtr, src);
    }
    
    public long[] decompressNumericLongs(byte[] src) {
        checkNotClosed();
        if (src == null) {
            throw new IllegalArgumentException("Source data cannot be null");
        }
        return OpenZLJNI.decompressNumericLongs(nativePtr, src);
    }
    
    public float[] decompressNumericFloats(byte[] src) {
        checkNotClosed();
        if (src == null) {
            throw new IllegalArgumentException("Source data cannot be null");
        }
        return OpenZLJNI.decompressNumericFloats(nativePtr, src);
    }
    
    public double[] decompressNumericDoubles(byte[] src) {
        checkNotClosed();
        if (src == null) {
            throw new IllegalArgumentException("Source data cannot be null");
        }
        return OpenZLJNI.decompressNumericDoubles(nativePtr, src);
    }
    
    public CompressionInfo getInfo(byte[] src) {
        if (src == null) {
            throw new IllegalArgumentException("Source data cannot be null");
        }
        return OpenZLJNI.getCompressionInfo(src);
    }
    
    public void close() {
        if (!closed) {
            closed = true;
            OpenZLJNI.destroyDecompressor(nativePtr);
        }
    }
    
    private void checkNotClosed() {
        if (closed) {
            throw new IllegalStateException("Decompressor has been closed");
        }
    }
    

    
    private static void checkRange(byte[] buf, int off, int len) {
        if (off < 0 || len < 0 || off + len > buf.length) {
            throw new IndexOutOfBoundsException("Invalid range: offset=" + off + ", length=" + len + ", buffer length=" + buf.length);
        }
    }
}