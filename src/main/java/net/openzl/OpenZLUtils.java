package net.openzl;

import java.nio.charset.StandardCharsets;

public final class OpenZLUtils {
    
    private OpenZLUtils() {
    }
    
    public static byte[] compressString(String text) {
        if (text == null) {
            throw new IllegalArgumentException("Text cannot be null");
        }
        
        try (OpenZLCompressor compressor = OpenZLFactory.fastCompressor()) {
            return compressor.compress(text.getBytes(StandardCharsets.UTF_8));
        }
    }
    
    public static String decompressString(byte[] compressedData) {
        if (compressedData == null) {
            throw new IllegalArgumentException("Compressed data cannot be null");
        }
        
        try (OpenZLDecompressor decompressor = OpenZLFactory.fastDecompressor()) {
            byte[] decompressed = decompressor.decompress(compressedData);
            return new String(decompressed, StandardCharsets.UTF_8);
        }
    }
    
    public static byte[] compressBinary(byte[] data) {
        if (data == null) {
            throw new IllegalArgumentException("Data cannot be null");
        }
        
        try (OpenZLCompressor compressor = OpenZLFactory.fastCompressor()) {
            return compressor.compress(data);
        }
    }
    
    public static byte[] decompressBinary(byte[] compressedData) {
        if (compressedData == null) {
            throw new IllegalArgumentException("Compressed data cannot be null");
        }
        
        try (OpenZLDecompressor decompressor = OpenZLFactory.fastDecompressor()) {
            return decompressor.decompress(compressedData);
        }
    }
    
    public static byte[] compressInts(int[] data) {
        if (data == null) {
            throw new IllegalArgumentException("Data cannot be null");
        }
        
        try (OpenZLCompressor compressor = OpenZLFactory.fastCompressor()) {
            return compressor.compressInts(data);
        }
    }
    
    public static int[] decompressInts(byte[] compressedData) {
        if (compressedData == null) {
            throw new IllegalArgumentException("Compressed data cannot be null");
        }
        
        try (OpenZLDecompressor decompressor = OpenZLFactory.fastDecompressor()) {
            return decompressor.decompressNumericInts(compressedData);
        }
    }
    
    public static byte[] compressLongs(long[] data) {
        if (data == null) {
            throw new IllegalArgumentException("Data cannot be null");
        }
        
        try (OpenZLCompressor compressor = OpenZLFactory.fastCompressor()) {
            return compressor.compressLongs(data);
        }
    }
    
    public static long[] decompressLongs(byte[] compressedData) {
        if (compressedData == null) {
            throw new IllegalArgumentException("Compressed data cannot be null");
        }
        
        try (OpenZLDecompressor decompressor = OpenZLFactory.fastDecompressor()) {
            return decompressor.decompressNumericLongs(compressedData);
        }
    }
    
    public static byte[] compressFloats(float[] data) {
        if (data == null) {
            throw new IllegalArgumentException("Data cannot be null");
        }
        
        try (OpenZLCompressor compressor = OpenZLFactory.fastCompressor()) {
            return compressor.compressFloats(data);
        }
    }
    
    public static float[] decompressFloats(byte[] compressedData) {
        if (compressedData == null) {
            throw new IllegalArgumentException("Compressed data cannot be null");
        }
        
        try (OpenZLDecompressor decompressor = OpenZLFactory.fastDecompressor()) {
            return decompressor.decompressNumericFloats(compressedData);
        }
    }
    
    public static byte[] compressDoubles(double[] data) {
        if (data == null) {
            throw new IllegalArgumentException("Data cannot be null");
        }
        
        try (OpenZLCompressor compressor = OpenZLFactory.fastCompressor()) {
            return compressor.compressDoubles(data);
        }
    }
    
    public static double[] decompressDoubles(byte[] compressedData) {
        if (compressedData == null) {
            throw new IllegalArgumentException("Compressed data cannot be null");
        }
        
        try (OpenZLDecompressor decompressor = OpenZLFactory.fastDecompressor()) {
            return decompressor.decompressNumericDoubles(compressedData);
        }
    }
    
    public static CompressionInfo getCompressionInfo(byte[] compressedData) {
        if (compressedData == null) {
            throw new IllegalArgumentException("Compressed data cannot be null");
        }
        
        try (OpenZLDecompressor decompressor = OpenZLFactory.fastDecompressor()) {
            return decompressor.getInfo(compressedData);
        }
    }
    
    public static int estimateMaxCompressedSize(int originalSize) {
        try (OpenZLCompressor compressor = OpenZLFactory.fastCompressor()) {
            return compressor.maxCompressedLength(originalSize);
        }
    }
}