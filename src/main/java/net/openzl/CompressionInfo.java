package net.openzl;

public final class CompressionInfo {
    
    private final long originalSize;
    private final long compressedSize;
    private final CompressionGraph graph;
    private final String dataType;
    
    public CompressionInfo(long originalSize, long compressedSize, CompressionGraph graph, String dataType) {
        this.originalSize = originalSize;
        this.compressedSize = compressedSize;
        this.graph = graph;
        this.dataType = dataType;
    }
    
    public long getOriginalSize() {
        return originalSize;
    }
    
    public long getCompressedSize() {
        return compressedSize;
    }
    
    public CompressionGraph getGraph() {
        return graph;
    }
    
    public String getDataType() {
        return dataType;
    }
    
    public double getCompressionRatio() {
        if (originalSize == 0) {
            return 0.0;
        }
        return (double) compressedSize / originalSize;
    }
    
    @Override
    public String toString() {
        return String.format("CompressionInfo{originalSize=%d, compressedSize=%d, graph=%s, dataType='%s', ratio=%.2f%%}",
                originalSize, compressedSize, graph, dataType, getCompressionRatio() * 100.0);
    }
}