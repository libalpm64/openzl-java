package net.openzl;

public final class OpenZLFactory {
    
    private static final OpenZLFactory FASTEST_INSTANCE = new OpenZLFactory();
    private static final OpenZLFactory SAFE_INSTANCE = new OpenZLFactory();
    
    static {
        OpenZLJNI.init();
    }
    
    public static OpenZLFactory fastestInstance() {
        return FASTEST_INSTANCE;
    }
    
    public static OpenZLFactory safeInstance() {
        return SAFE_INSTANCE;
    }
    
    public static OpenZLCompressor fastCompressor() {
        init();
        return new OpenZLCompressor(CompressionGraph.ZSTD);
    }
    
    public static OpenZLCompressor highCompressor() {
        init();
        return new OpenZLCompressor(CompressionGraph.ZSTD);
    }
    
    public static OpenZLCompressor compressor(CompressionGraph graph) {
        init();
        return new OpenZLCompressor(graph);
    }
    
    public static OpenZLDecompressor fastDecompressor() {
        init();
        return new OpenZLDecompressor();
    }
    
    public static OpenZLDecompressor safeDecompressor() {
        init();
        return new OpenZLDecompressor();
    }
    
    private static void init() {
        OpenZLJNI.init();
    }
    
    private OpenZLFactory() {
    }
}