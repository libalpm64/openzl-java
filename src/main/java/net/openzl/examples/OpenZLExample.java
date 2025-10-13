package net.openzl.examples;

import net.openzl.*;
import java.util.Random;

public class OpenZLExample {

    private static final String TEXT = 
        "Sophocles and Dionysius the tyrant, who passed away for gladness, and Talva, who died in Corsica " +
        "reading the news of the honors that the Senate of Rome had decreed for him, we understand that in our " +
        "time Pope Leo X, upon being advised of the taking of Milan, which he had ardently desired, fell into " +
        "such an excess of joy that a fever got him and he died. And as a more remarkable testimony of human " +
        "frailty, it was noted by the ancients that Diodorus the dialectician died on the spot, seized with " +
        "an extreme passion of shame, for not having been able to shake loose, in his own school and in public, " +
        "from an argument that had been put to him.";

    public static void main(String[] args) {
        System.out.println("OpenZL Example:");

        try {
            byte[] orig = TEXT.getBytes();
            try (var c = OpenZLFactory.fastCompressor(); var d = OpenZLFactory.fastDecompressor()) {
                byte[] comp = c.compress(orig);
                byte[] decomp = d.decompress(comp);
                if (!java.util.Arrays.equals(orig, decomp))
                    throw new RuntimeException("Corruption!");
                System.out.printf("Original: %s → Compressed: %s (%.2fx) success%n",
                    fmt(orig.length), fmt(comp.length), (double) orig.length / comp.length);
            }
            byte[] test = "test".getBytes();
            int fastSz, highSz;
            try (var f = OpenZLFactory.fastCompressor()) { fastSz = f.compress(test).length; }
            try (var h = OpenZLFactory.highCompressor()) { highSz = h.compress(test).length; }
            System.out.printf("Fast: %s → %s (%.2fx) | High: %s → %s (%.2fx) success%n",
                fmt(test.length), fmt(fastSz), (double)test.length/fastSz,
                fmt(test.length), fmt(highSz), (double)test.length/highSz);

            System.out.println("Starting benchmarks...");
            for (int multiplier : new int[]{1, 5, 10}) {
                try {
                    StringBuilder sb = new StringBuilder();
                    for (int i = 0; i < multiplier; i++) {
                        sb.append(TEXT).append(" ");
                    }
                    byte[] data = sb.toString().getBytes();
                    
                    System.out.printf("Benchmarking %s text data...\n", fmt(data.length));
                    
                    System.out.println("  Warming up...");
                    for (int i = 0; i < 3; i++) {
                        try (var comp = OpenZLFactory.fastCompressor(); var decomp = OpenZLFactory.fastDecompressor()) {
                            byte[] compressed = comp.compress(data);
                            decomp.decompress(compressed);
                        }
                    }
                    
                    System.out.println("  Running benchmark...");
                    long t0 = System.nanoTime();
                    byte[] c;
                    try (var comp = OpenZLFactory.fastCompressor()) {
                        c = comp.compress(data);
                    }
                    long t1 = System.nanoTime();
                    try (var decomp = OpenZLFactory.fastDecompressor()) {
                        decomp.decompress(c);
                    }
                    long t2 = System.nanoTime();
                    double ratio = (double) data.length / c.length;
                    System.out.printf("  Result: %.2f ms comp / %.2f ms decomp @ %.1fx%n",
                        (t1 - t0) / 1_000_000.0,
                        (t2 - t1) / 1_000_000.0,
                        ratio);
                } catch (Exception e) {
                    System.err.printf("Error during benchmark for multiplier %d: %s%n", multiplier, e.getMessage());
                    e.printStackTrace();
                }
            }

            System.out.println("success all operations completed sucessfully!");

        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
            System.exit(1);
        }
    }

    static byte[] genData(int n) {
        byte[] b = new byte[n];
        Random r = new Random(42);
        for (int i = 0; i < n; i++) b[i] = (byte) (i % 100 < 80 ? 'A' + (i % 26) : r.nextInt(256));
        return b;
    }

    static String fmt(int bytes) {
        if (bytes < 1024) return bytes + " B";
        if (bytes < 1024 * 1024) return String.format("%.1f KB", bytes / 1024.0);
        return String.format("%.1f MB", bytes / (1024.0 * 1024.0));
    }
}