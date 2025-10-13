package net.openzl;

import java.io.*;
import java.nio.file.*;

final class NativeLibraryLoader {
    
    private static final String NATIVE_FOLDER_PATH_PREFIX = "native";
    
    static void loadLibrary(String libraryName) throws IOException {
        String platformLibraryName = getPlatformLibraryName(libraryName);
        String resourcePath = "/" + NATIVE_FOLDER_PATH_PREFIX + "/" + getOSName() + "/" + getArchName() + "/" + platformLibraryName;
        
        InputStream libraryStream = NativeLibraryLoader.class.getResourceAsStream(resourcePath);
        if (libraryStream == null) {
            throw new IOException("Native library not found in resources: " + resourcePath);
        }
        
        try {
            Path tempDir = Files.createTempDirectory("openzl_native");
            tempDir.toFile().deleteOnExit();
            
            Path tempLibrary = tempDir.resolve(platformLibraryName);
            tempLibrary.toFile().deleteOnExit();
            
            Files.copy(libraryStream, tempLibrary, StandardCopyOption.REPLACE_EXISTING);
            System.load(tempLibrary.toAbsolutePath().toString());
            
        } finally {
            libraryStream.close();
        }
    }
    
    private static String getPlatformLibraryName(String libraryName) {
        String osName = getOSName();
        
        switch (osName) {
            case "windows":
                return libraryName + ".dll";
            case "linux":
                return "lib" + libraryName + ".so";
            case "macos":
                return "lib" + libraryName + ".dylib";
            default:
                throw new UnsupportedOperationException("Unsupported operating system: " + osName);
        }
    }
    
    private static String getOSName() {
        String osName = System.getProperty("os.name").toLowerCase();
        
        if (osName.contains("windows")) {
            return "windows";
        } else if (osName.contains("linux")) {
            return "linux";
        } else if (osName.contains("mac") || osName.contains("darwin")) {
            return "macos";
        } else {
            throw new UnsupportedOperationException("Unsupported operating system: " + osName);
        }
    }
    
    private static String getArchName() {
        String archName = System.getProperty("os.arch").toLowerCase();
        
        if (archName.contains("amd64") || archName.contains("x86_64")) {
            return "x86_64";
        } else if (archName.contains("x86") || archName.contains("i386")) {
            return "x86";
        } else if (archName.contains("aarch64") || archName.contains("arm64")) {
            return "aarch64";
        } else if (archName.contains("arm")) {
            return "arm";
        } else {
            throw new UnsupportedOperationException("Unsupported architecture: " + archName);
        }
    }
    
    private NativeLibraryLoader() {
    }
}