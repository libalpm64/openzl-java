package net.openzl;

public enum CompressionGraph {
    
    ZSTD(0),
    NUMERIC(1),
    FIELD_LZ(2),
    STORE(3),
    FSE(4),
    HUFFMAN(5),
    ENTROPY(6),
    BITPACK(7),
    CONSTANT(8),
    SERIAL_COMPRESS(9),
    CSV(10),
    SDDL(11),
    PARQUET(12);
    
    private final int id;
    
    CompressionGraph(int id) {
        this.id = id;
    }
    
    public int getId() {
        return id;
    }
    
    public static CompressionGraph fromId(int id) {
        for (CompressionGraph graph : values()) {
            if (graph.id == id) {
                return graph;
            }
        }
        throw new IllegalArgumentException("Invalid compression graph ID: " + id);
    }
}