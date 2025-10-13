package net.openzl;

import java.nio.ByteBuffer;

public final class OpenZLCompressor implements AutoCloseable {
    
    private final CompressionGraph graph;
    private final long nativePtr;
    private volatile boolean closed = false;
    
    OpenZLCompressor(CompressionGraph graph) {
        this.graph = graph;
        this.nativePtr = OpenZLJNI.createCompressor(graph.getId());
        if (this.nativePtr == 0) {
            throw new OpenZLException("Failed to create compressor");
        }
    }
    
    public byte[] compress(byte[] src) {
        return compress(src, 0, src.length);
    }
    
    public byte[] compress(byte[] src, int srcOff, int srcLen) {
        checkNotClosed();
        if (src == null) {
            throw new IllegalArgumentException("Source array cannot be null");
        }
        if (srcOff < 0 || srcLen < 0 || srcOff + srcLen > src.length) {
            throw new IndexOutOfBoundsException("Invalid offset or length");
        }
        
        return OpenZLJNI.compressSerial(nativePtr, src, srcOff, srcLen);
    }
    
    public int compress(byte[] src, int srcOff, int srcLen, byte[] dest, int destOff, int maxDestLen) {
        if (src == null || dest == null) {
            throw new IllegalArgumentException("Source and destination cannot be null");
        }
        checkRange(src, srcOff, srcLen);
        checkRange(dest, destOff, maxDestLen);
        return OpenZLJNI.compressSerialToBuffer(nativePtr, src, srcOff, srcLen, dest, destOff, maxDestLen);
    }
    
    public byte[] compress(ByteBuffer src) {
        checkNotClosed();
        if (src == null) {
            throw new IllegalArgumentException("Source buffer cannot be null");
        }
        
        if (src.hasArray()) {
            return compress(src.array(), src.arrayOffset() + src.position(), src.remaining());
        } else {
            byte[] data = new byte[src.remaining()];
            src.get(data);
            return compress(data);
        }
    }
    
    public int compress(ByteBuffer src, ByteBuffer dest) {
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
        
        int written = compress(srcArray, srcOff, srcLen, destArray, destOff, maxDestLen);
        
        if (!dest.hasArray()) {
            dest.put(destArray, 0, written);
        } else {
            dest.position(dest.position() + written);
        }
        
        return written;
    }
    
    public byte[] compressNumeric(byte[] data, int elementSize, int elementCount) {
        if (data == null) {
            throw new IllegalArgumentException("Data cannot be null");
        }
        if (elementSize <= 0 || elementCount <= 0) {
            throw new IllegalArgumentException("Element size and count must be positive");
        }
        if (data.length != elementSize * elementCount) {
            throw new IllegalArgumentException("Data length doesn't match element size * count");
        }
        return OpenZLJNI.compressNumeric(nativePtr, data, elementSize, elementCount);
    }
    
    public byte[] compressInts(int[] data) {
        checkNotClosed();
        if (data == null) {
            throw new IllegalArgumentException("Data array cannot be null");
        }
        return OpenZLJNI.compressNumericInts(nativePtr, data);
    }
    
    public byte[] compressLongs(long[] data) {
        checkNotClosed();
        if (data == null) {
            throw new IllegalArgumentException("Data array cannot be null");
        }
        return OpenZLJNI.compressNumericLongs(nativePtr, data);
    }
    
    public byte[] compressFloats(float[] data) {
        checkNotClosed();
        if (data == null) {
            throw new IllegalArgumentException("Data array cannot be null");
        }
        return OpenZLJNI.compressNumericFloats(nativePtr, data);
    }
    
    public byte[] compressDoubles(double[] data) {
        checkNotClosed();
        if (data == null) {
            throw new IllegalArgumentException("Data array cannot be null");
        }
        return OpenZLJNI.compressNumericDoubles(nativePtr, data);
    }
    
    public static int maxCompressedLength(int srcLen) {
        return OpenZLJNI.compressBound(srcLen);
    }
    
    public CompressionGraph getGraph() {
        return graph;
    }
    
    public void close() {
        if (!closed) {
            closed = true;
            if (nativePtr != 0) {
                OpenZLJNI.destroyCompressor(nativePtr);
            }
        }
    }
    
    private void checkNotClosed() {
        if (closed) {
            throw new IllegalStateException("Compressor has been closed");
        }
    }
    

    
    private static void checkRange(byte[] buf, int off, int len) {
        if (off < 0 || len < 0 || off + len > buf.length) {
            throw new IndexOutOfBoundsException("Invalid range: offset=" + off + ", length=" + len + ", buffer length=" + buf.length);
        }
    }
}