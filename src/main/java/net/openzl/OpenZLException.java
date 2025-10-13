package net.openzl;

public class OpenZLException extends RuntimeException {
    
    private final int errorCode;
    private final String errorName;
    
    public OpenZLException(String message) {
        super(message);
        this.errorCode = -1;
        this.errorName = "Unknown";
    }
    
    public OpenZLException(String message, Throwable cause) {
        super(message, cause);
        this.errorCode = -1;
        this.errorName = "Unknown";
    }
    
    public OpenZLException(int errorCode, String errorName, String message) {
        super(String.format("OpenZL error: %d (%s) %s", errorCode, errorName, message));
        this.errorCode = errorCode;
        this.errorName = errorName;
    }
    
    public int getErrorCode() {
        return errorCode;
    }
    
    public String getErrorName() {
        return errorName;
    }
}