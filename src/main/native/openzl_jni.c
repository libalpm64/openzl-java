// SPDX-License-Identifier: MIT
// OpenZL JNI Bindings
// Copyright (c) 2025 Lostlab Technologies
// 
// Licensed under the MIT License.
// You may use, modify, and distribute this code freely, provided that this notice is retained.
//
// Note: OpenZL is a compression framework owned and copyrighted by 
// Meta Platforms, Inc. All rights to OpenZL itself are reserved by Meta.
//
// This file only provides JNI bindings for OpenZL and is not affiliated with Meta.

#include <jni.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "openzl.h"

static void throw_exception(JNIEnv *env, const char *exception_class, const char *message) {
    jclass exc_class = (*env)->FindClass(env, exception_class);
    if (exc_class != NULL) {
        (*env)->ThrowNew(env, exc_class, message);
    }
}

static void throw_openzl_exception(JNIEnv *env, const char *message) {
    throw_exception(env, "net/openzl/OpenZLException", message);
}

static void throw_out_of_memory(JNIEnv *env) {
    throw_exception(env, "java/lang/OutOfMemoryError", "Failed to allocate memory");
}

static void throw_openzl_report_error(JNIEnv *env, ZL_Report report) {
    if (ZL_isError(report)) {
        ZL_ErrorCode error_code = ZL_errorCode(report);
        const char *error_message = ZL_ErrorCode_toString(error_code);
        throw_openzl_exception(env, error_message);
    }
}

/**
 * Helper function to safely release JNI array elements and free allocated memory.
 * This reduces code duplication in error handling paths.
 */
static void cleanup_int_array_and_buffer(JNIEnv *env, jintArray array, jint *array_elements, void *buffer) {
    if (array_elements != NULL && array != NULL) {
        (*env)->ReleaseIntArrayElements(env, array, array_elements, JNI_ABORT);
    }
    if (buffer != NULL) {
        free(buffer);
    }
}

static void cleanup_byte_array_and_buffer(JNIEnv *env, jbyteArray array, jbyte *array_elements, void *buffer) {
    if (array_elements != NULL && array != NULL) {
        (*env)->ReleaseByteArrayElements(env, array, array_elements, JNI_ABORT);
    }
    if (buffer != NULL) {
        free(buffer);
    }
}

/**
 * Helper function to safely free OpenZL typed reference.
 */
static void cleanup_typed_ref(ZL_TypedRef *typed_ref) {
    if (typed_ref != NULL) {
        ZL_TypedRef_free(typed_ref);
    }
}

typedef struct {
    ZL_CCtx *ctx;
    ZL_Compressor *compressor;
    ZL_GraphID graph_id;
} openzl_compressor_t;

typedef struct {
    ZL_DCtx *ctx;
} openzl_decompressor_t;

JNIEXPORT void JNICALL
Java_net_openzl_OpenZLJNI_nativeInit(JNIEnv *env, jclass clazz) {
    // Currently OpenZL doesn't require explicit initialization
}

/**
 * Shuts down the OpenZL native library and performs cleanup operations.
 * 
 * This method provides symmetry with nativeInit() and allows for future cleanup
 * operations if OpenZL introduces global state that needs to be released.
 * Currently, OpenZL doesn't require explicit shutdown, but this method is
 * provided for future-proofing and API completeness.
 */
 
JNIEXPORT void JNICALL
Java_net_openzl_OpenZLJNI_nativeShutdown(JNIEnv *env, jclass clazz) {
    // Currently OpenZL doesn't require explicit shutdown
}

/**
 * Creates a new OpenZL compressor configured with the specified built-in compression graph.
 * 
 * Graph ID mappings:
 *   0, default: ZSTD (fallback)
 *   1, 9       : Generic compressor (ZL_GRAPH_COMPRESS_GENERIC)
 *   2          : FieldLZ (numeric/structured data)
 *   3          : Store (no compression)
 *   4          : FSE entropy coding
 *   5          : Huffman coding
 *   6          : General entropy coding
 *   7          : Bitpacking
 *   8          : Constant-value optimization
 *
 * Returns a native pointer to the compressor; caller must call destroyCompressor() to free it.
 */

JNIEXPORT jlong JNICALL
Java_net_openzl_OpenZLJNI_createCompressor(JNIEnv *env, jclass clazz, jint graph_id) {
    openzl_compressor_t *compressor = malloc(sizeof(openzl_compressor_t));
    if (compressor == NULL) {
        throw_out_of_memory(env);
        return 0;
    }
    
    compressor->ctx = ZL_CCtx_create();
    if (compressor->ctx == NULL) {
        free(compressor);
        throw_openzl_exception(env, "Failed to create compression context");
        return 0;
    }
    
    compressor->compressor = ZL_Compressor_create();
    if (compressor->compressor == NULL) {
        ZL_CCtx_free(compressor->ctx);
        free(compressor);
        throw_openzl_exception(env, "Failed to create compressor object");
        return 0;
    }
    
    ZL_GraphID selected_graph;
    switch (graph_id) {
        case 0:
            selected_graph = ZL_GRAPH_ZSTD;
            break;
        case 1:
            selected_graph = ZL_GRAPH_COMPRESS_GENERIC;
            break;
        case 2:
            selected_graph = ZL_GRAPH_FIELD_LZ;
            break;
        case 3:
            selected_graph = ZL_GRAPH_STORE;
            break;  
        case 4:
            selected_graph = ZL_GRAPH_FSE;
            break;
        case 5:
            selected_graph = ZL_GRAPH_HUFFMAN;
            break;
        case 6:
            selected_graph = ZL_GRAPH_ENTROPY;
            break;
        case 7:
            selected_graph = ZL_GRAPH_BITPACK;
            break;
        case 8:
            selected_graph = ZL_GRAPH_CONSTANT;
            break;
        case 9:
            selected_graph = ZL_GRAPH_COMPRESS_GENERIC;
            break;
        default:
            selected_graph = ZL_GRAPH_ZSTD;
            break;
    }
    
    compressor->graph_id = selected_graph;
    
    ZL_Report result = ZL_CCtx_setParameter(compressor->ctx, ZL_CParam_formatVersion, ZL_MAX_FORMAT_VERSION);
    if (ZL_isError(result)) {
        ZL_Compressor_free(compressor->compressor);
        ZL_CCtx_free(compressor->ctx);
        free(compressor);
        throw_openzl_report_error(env, result);
        return 0;
    }
    
    result = ZL_CCtx_setParameter(compressor->ctx, ZL_CParam_compressionLevel, ZL_COMPRESSIONLEVEL_DEFAULT);
    if (ZL_isError(result)) {
        ZL_Compressor_free(compressor->compressor);
        ZL_CCtx_free(compressor->ctx);
        free(compressor);
        throw_openzl_report_error(env, result);
        return 0;
    }
    
    result = ZL_Compressor_selectStartingGraphID(compressor->compressor, selected_graph);
    if (ZL_isError(result)) {
        ZL_Compressor_free(compressor->compressor);
        ZL_CCtx_free(compressor->ctx);
        free(compressor);
        throw_openzl_report_error(env, result);
        return 0;
    }
    
    result = ZL_CCtx_refCompressor(compressor->ctx, compressor->compressor);
    if (ZL_isError(result)) {
        ZL_Compressor_free(compressor->compressor);
        ZL_CCtx_free(compressor->ctx);
        free(compressor);
        throw_openzl_report_error(env, result);
        return 0;
    }
    
    return (jlong)(uintptr_t)compressor;
}

/**
 * Frees a compressor instance created by createCompressor().
 */
JNIEXPORT void JNICALL
Java_net_openzl_OpenZLJNI_destroyCompressor(JNIEnv *env, jclass clazz, jlong compressor_ptr) {
    if (compressor_ptr == 0) return;
    
    openzl_compressor_t *compressor = (openzl_compressor_t *)(uintptr_t)compressor_ptr;
    if (compressor->compressor != NULL) {
        ZL_Compressor_free(compressor->compressor);
    }
    if (compressor->ctx != NULL) {
        ZL_CCtx_free(compressor->ctx);
    }
    free(compressor);
}

/**
 * Creates a new OpenZL decompressor instance.
 * Caller must call destroyDecompressor() to free the returned resource.
 */
JNIEXPORT jlong JNICALL
Java_net_openzl_OpenZLJNI_createDecompressor(JNIEnv *env, jclass clazz) {
    openzl_decompressor_t *decompressor = malloc(sizeof(openzl_decompressor_t));
    if (decompressor == NULL) {
        throw_out_of_memory(env);
        return 0;
    }
    
    decompressor->ctx = ZL_DCtx_create();
    if (decompressor->ctx == NULL) {
        free(decompressor);
        throw_openzl_exception(env, "Failed to create decompression context");
        return 0;
    }
    
    return (jlong)(uintptr_t)decompressor;
}

/**
 * Frees a decompressor instance created by createDecompressor().
 */
JNIEXPORT void JNICALL
Java_net_openzl_OpenZLJNI_destroyDecompressor(JNIEnv *env, jclass clazz, jlong decompressor_ptr) {
    if (decompressor_ptr == 0) return;
    
    openzl_decompressor_t *decompressor = (openzl_decompressor_t *)(uintptr_t)decompressor_ptr;
    if (decompressor->ctx != NULL) {
        ZL_DCtx_free(decompressor->ctx);
    }
    free(decompressor);
}

/**
 * Compresses a segment of byte data and returns a new byte array with the compressed result.
 * Returns null and throws an exception on failure.
 * The compressor must not be shared across threads while in use.
 */
JNIEXPORT jbyteArray JNICALL
Java_net_openzl_OpenZLJNI_compressSerial(JNIEnv *env, jclass clazz, jlong compressor_ptr,
                                        jbyteArray src, jint src_off, jint src_len) {
    if (compressor_ptr == 0) {
        throw_openzl_exception(env, "[Error OpenZL JNI] Invalid compressor");
        return NULL;
    }
    
    openzl_compressor_t *compressor = (openzl_compressor_t *)(uintptr_t)compressor_ptr;
    
    jbyte *src_data = (*env)->GetByteArrayElements(env, src, NULL);
    if (src_data == NULL) {
        throw_out_of_memory(env);
        return NULL;
    }
    
    size_t max_compressed_size = ZL_compressBound(src_len);
    
    void *compressed_data = malloc(max_compressed_size);
    if (compressed_data == NULL) {
        (*env)->ReleaseByteArrayElements(env, src, src_data, JNI_ABORT);
        throw_out_of_memory(env);
        return NULL;
    }
    
    ZL_TypedRef* typed_ref = ZL_TypedRef_createSerial(src_data + src_off, src_len);
    if (typed_ref == NULL) {
        (*env)->ReleaseByteArrayElements(env, src, src_data, JNI_ABORT);
        free(compressed_data);
        throw_openzl_exception(env, "[Error OpenZL JNI] Failed to create typed reference for serial data");
        return NULL;
    }

    ZL_Report compress_report = ZL_CCtx_compressTypedRef(
        compressor->ctx,
        compressed_data, max_compressed_size,
        typed_ref
    );

    ZL_TypedRef_free(typed_ref);
    
    (*env)->ReleaseByteArrayElements(env, src, src_data, JNI_ABORT);
    
    if (ZL_isError(compress_report)) {
        free(compressed_data);
        throw_openzl_report_error(env, compress_report);
        return NULL;
    }
    
    size_t compressed_size = ZL_validResult(compress_report);
    
    jbyteArray result = (*env)->NewByteArray(env, compressed_size);
    if (result == NULL) {
        free(compressed_data);
        throw_out_of_memory(env);
        return NULL;
    }
    
    (*env)->SetByteArrayRegion(env, result, 0, compressed_size, (jbyte *)compressed_data);
    free(compressed_data);
    
    return result;
}

/**
 * Compresses a segment of byte data directly into a pre-allocated destination buffer.
 * Returns the actual compressed size on success, or -1 if an error occurs (exception thrown).
 * The destination buffer must be large enough—use compressBound() to determine required size.
 */
JNIEXPORT jint JNICALL
Java_net_openzl_OpenZLJNI_compressSerialToBuffer(JNIEnv *env, jclass clazz, jlong compressor_ptr,
                                                jbyteArray src, jint src_off, jint src_len,
                                                jbyteArray dest, jint dest_off, jint max_dest_len) {
    if (compressor_ptr == 0) {
        throw_openzl_exception(env, "[Error OpenZL JNI] Invalid compressor");
        return -1;
    }
    
    openzl_compressor_t *compressor = (openzl_compressor_t *)(uintptr_t)compressor_ptr;
    
    jbyte *src_data = (*env)->GetByteArrayElements(env, src, NULL);
    jbyte *dest_data = (*env)->GetByteArrayElements(env, dest, NULL);
    
    if (src_data == NULL || dest_data == NULL) {
        if (src_data) (*env)->ReleaseByteArrayElements(env, src, src_data, JNI_ABORT);
        if (dest_data) (*env)->ReleaseByteArrayElements(env, dest, dest_data, JNI_ABORT);
        throw_out_of_memory(env);
        return -1;
    }
    
    ZL_TypedRef* typed_ref = ZL_TypedRef_createSerial(src_data + src_off, src_len);
    if (typed_ref == NULL) {
        (*env)->ReleaseByteArrayElements(env, src, src_data, JNI_ABORT);
        (*env)->ReleaseByteArrayElements(env, dest, dest_data, JNI_ABORT);
        throw_openzl_exception(env, "[Error OpenZL JNI] Failed to create typed reference");
        return -1;
    }

    ZL_Report compress_report = ZL_CCtx_compressTypedRef(
        compressor->ctx,
        dest_data + dest_off, max_dest_len,
        typed_ref
    );

    ZL_TypedRef_free(typed_ref);
    
    (*env)->ReleaseByteArrayElements(env, src, src_data, JNI_ABORT);
    (*env)->ReleaseByteArrayElements(env, dest, dest_data, 0);
    
    if (ZL_isError(compress_report)) {
        throw_openzl_exception(env, ZL_ErrorCode_toString(ZL_errorCode(compress_report)));
        return -1;
    }
    
    size_t compressed_size = ZL_validResult(compress_report);
    return (jint)compressed_size;
}

/**
 * Compresses a Java int array using OpenZL's numeric compression pipeline.
 * Treats the data as 32-bit signed integers for improved compression efficiency.
 * Returns a new byte array containing the compressed data, or throws an exception on failure.
 */
JNIEXPORT jbyteArray JNICALL
Java_net_openzl_OpenZLJNI_compressNumericInts(JNIEnv *env, jclass clazz, jlong compressor_ptr, jintArray data) {
    if (compressor_ptr == 0) {
        throw_openzl_exception(env, "[Error OpenZL JNI] Invalid compressor");
        return NULL;
    }
    
    openzl_compressor_t *compressor = (openzl_compressor_t *)(uintptr_t)compressor_ptr;
    
    jsize array_len = (*env)->GetArrayLength(env, data);
    jint *array_data = (*env)->GetIntArrayElements(env, data, NULL);
    
    if (array_data == NULL) {
        throw_out_of_memory(env);
        return NULL;
    }
    
    size_t input_size = array_len * sizeof(jint);
    size_t max_compressed_size = ZL_compressBound(input_size);
    
    void *compressed_data = malloc(max_compressed_size);
    if (compressed_data == NULL) {
        (*env)->ReleaseIntArrayElements(env, data, array_data, JNI_ABORT);
        throw_out_of_memory(env);
        return NULL;
    }
    
    ZL_TypedRef* typed_ref = ZL_TypedRef_createNumeric(array_data, sizeof(jint), array_len);
    if (typed_ref == NULL) {
        (*env)->ReleaseIntArrayElements(env, data, array_data, JNI_ABORT);
        free(compressed_data);
        throw_openzl_exception(env, "[Error OpenZL JNI] Failed to create typed reference for numeric data");
        return NULL;
    }
    
    ZL_Report compress_report = ZL_CCtx_compressTypedRef(
        compressor->ctx,
        compressed_data, max_compressed_size,
        typed_ref
    );
    
    ZL_TypedRef_free(typed_ref);
    (*env)->ReleaseIntArrayElements(env, data, array_data, JNI_ABORT);
    
    if (ZL_isError(compress_report)) {
        free(compressed_data);
        throw_openzl_report_error(env, compress_report);
        return NULL;
    }
    
    size_t compressed_size = ZL_validResult(compress_report);
    
    jbyteArray result = (*env)->NewByteArray(env, compressed_size);
    if (result == NULL) {
        free(compressed_data);
        throw_out_of_memory(env);
        return NULL;
    }
    
    (*env)->SetByteArrayRegion(env, result, 0, compressed_size, (jbyte *)compressed_data);
    free(compressed_data);
    
    return result;
}

/**
 * Compresses a Java long array using OpenZL's numeric compression pipeline.
 * Treats the data as an array of 64-bit signed integers (jlong) for optimized compression.
 * Returns a new byte array with the compressed data, or throws an exception on failure.
 */
JNIEXPORT jbyteArray JNICALL
Java_net_openzl_OpenZLJNI_compressNumericLongs(JNIEnv *env, jclass clazz, jlong compressor_ptr, jlongArray data) {
    if (compressor_ptr == 0) {
        throw_openzl_exception(env, "[Error OpenZL JNI]  Invalid compressor");
        return NULL;
    }
    
    openzl_compressor_t *compressor = (openzl_compressor_t *)(uintptr_t)compressor_ptr;
    
    jsize array_len = (*env)->GetArrayLength(env, data);
    jlong *array_data = (*env)->GetLongArrayElements(env, data, NULL);
    
    if (array_data == NULL) {
        throw_out_of_memory(env);
        return NULL;
    }
    
    size_t input_size = array_len * sizeof(jlong);
    size_t max_compressed_size = ZL_compressBound(input_size);
    
    void *compressed_data = malloc(max_compressed_size);
    if (compressed_data == NULL) {
        (*env)->ReleaseLongArrayElements(env, data, array_data, JNI_ABORT);
        throw_out_of_memory(env);
        return NULL;
    }
    
    ZL_TypedRef* typed_ref = ZL_TypedRef_createNumeric(array_data, sizeof(jlong), array_len);
    if (typed_ref == NULL) {
        (*env)->ReleaseLongArrayElements(env, data, array_data, JNI_ABORT);
        free(compressed_data);
        throw_openzl_exception(env, "[Error OpenZL JNI] Failed to create typed reference for numeric data");
        return NULL;
    }
    
    ZL_Report compress_report = ZL_CCtx_compressTypedRef(
        compressor->ctx,
        compressed_data, max_compressed_size,
        typed_ref
    );
    
    ZL_TypedRef_free(typed_ref);
    (*env)->ReleaseLongArrayElements(env, data, array_data, JNI_ABORT);
    
    if (ZL_isError(compress_report)) {
        free(compressed_data);
        throw_openzl_report_error(env, compress_report);
        return NULL;
    }
    
    size_t compressed_size = ZL_validResult(compress_report);
    
    jbyteArray result = (*env)->NewByteArray(env, compressed_size);
    if (result == NULL) {
        free(compressed_data);
        throw_out_of_memory(env);
        return NULL;
    }
    
    (*env)->SetByteArrayRegion(env, result, 0, compressed_size, (jbyte *)compressed_data);
    free(compressed_data);
    
    return result;
}

/**
 * Compresses a Java float array by treating its binary representation as numeric data.
 * Returns a new byte array with compressed data, or throws an exception on failure.
 * Optimized for 32-bit floats using OpenZL's numeric compression pipeline.
 */
JNIEXPORT jbyteArray JNICALL
Java_net_openzl_OpenZLJNI_compressNumericFloats(JNIEnv *env, jclass clazz, jlong compressor_ptr, jfloatArray data) {
    if (compressor_ptr == 0) {
        throw_openzl_exception(env, "[Error OpenZL JNI] Invalid compressor");
        return NULL;
    }
    
    openzl_compressor_t *compressor = (openzl_compressor_t *)(uintptr_t)compressor_ptr;
    
    jsize array_len = (*env)->GetArrayLength(env, data);
    jfloat *array_data = (*env)->GetFloatArrayElements(env, data, NULL);
    
    if (array_data == NULL) {
        throw_out_of_memory(env);
        return NULL;
    }
    
    size_t input_size = array_len * sizeof(jfloat);
    size_t max_compressed_size = ZL_compressBound(input_size);
    
    void *compressed_data = malloc(max_compressed_size);
    if (compressed_data == NULL) {
        (*env)->ReleaseFloatArrayElements(env, data, array_data, JNI_ABORT);
        throw_out_of_memory(env);
        return NULL;
    }
    
    ZL_TypedRef* typed_ref = ZL_TypedRef_createNumeric(array_data, sizeof(jfloat), array_len);
    if (typed_ref == NULL) {
        (*env)->ReleaseFloatArrayElements(env, data, array_data, JNI_ABORT);
        free(compressed_data);
        throw_openzl_exception(env, "[Error OpenZL JNI] Failed to create typed reference for numeric data");
        return NULL;
    }
    
    ZL_Report compress_report = ZL_CCtx_compressTypedRef(
        compressor->ctx,
        compressed_data, max_compressed_size,
        typed_ref
    );
    
    ZL_TypedRef_free(typed_ref);
    (*env)->ReleaseFloatArrayElements(env, data, array_data, JNI_ABORT);
    
    if (ZL_isError(compress_report)) {
        free(compressed_data);
        throw_openzl_report_error(env, compress_report);
        return NULL;
    }
    
    size_t compressed_size = ZL_validResult(compress_report);
    
    jbyteArray result = (*env)->NewByteArray(env, compressed_size);
    if (result == NULL) {
        free(compressed_data);
        throw_out_of_memory(env);
        return NULL;
    }
    
    (*env)->SetByteArrayRegion(env, result, 0, compressed_size, (jbyte *)compressed_data);
    free(compressed_data);
    
    return result;
}

/**
 * Compresses a Java double array using OpenZL's numeric compression pipeline.
 * Treats the data as an array of 64-bit floating-point values for optimized compression.
 * Returns a new byte array with the compressed data, or throws an exception on failure.
 */
JNIEXPORT jbyteArray JNICALL
Java_net_openzl_OpenZLJNI_compressNumericDoubles(JNIEnv *env, jclass clazz, jlong compressor_ptr, jdoubleArray data) {
    if (compressor_ptr == 0) {
        throw_openzl_exception(env, "[Error OpenZL JNI] Invalid compressor");
        return NULL;
    }
    
    openzl_compressor_t *compressor = (openzl_compressor_t *)(uintptr_t)compressor_ptr;
    
    jsize array_len = (*env)->GetArrayLength(env, data);
    jdouble *array_data = (*env)->GetDoubleArrayElements(env, data, NULL);
    
    if (array_data == NULL) {
        throw_out_of_memory(env);
        return NULL;
    }
    
    size_t input_size = array_len * sizeof(jdouble);
    size_t max_compressed_size = ZL_compressBound(input_size);
    
    void *compressed_data = malloc(max_compressed_size);
    if (compressed_data == NULL) {
        (*env)->ReleaseDoubleArrayElements(env, data, array_data, JNI_ABORT);
        throw_out_of_memory(env);
        return NULL;
    }
    
    ZL_TypedRef* typed_ref = ZL_TypedRef_createNumeric(array_data, sizeof(jdouble), array_len);
    if (typed_ref == NULL) {
        (*env)->ReleaseDoubleArrayElements(env, data, array_data, JNI_ABORT);
        free(compressed_data);
        throw_openzl_exception(env, "[Error OpenZL JNI] Failed to create typed reference for numeric data");
        return NULL;
    }
    
    ZL_Report compress_report = ZL_CCtx_compressTypedRef(
        compressor->ctx,
        compressed_data, max_compressed_size,
        typed_ref
    );
    
    ZL_TypedRef_free(typed_ref);
    (*env)->ReleaseDoubleArrayElements(env, data, array_data, JNI_ABORT);
    
    if (ZL_isError(compress_report)) {
        free(compressed_data);
        throw_openzl_report_error(env, compress_report);
        return NULL;
    }
    
    size_t compressed_size = ZL_validResult(compress_report);
    
    jbyteArray result = (*env)->NewByteArray(env, compressed_size);
    if (result == NULL) {
        free(compressed_data);
        throw_out_of_memory(env);
        return NULL;
    }
    
    (*env)->SetByteArrayRegion(env, result, 0, compressed_size, (jbyte *)compressed_data);
    free(compressed_data);
    
    return result;
}

/**
 * Decompresses serial (byte array) data compressed with OpenZL.
 * Returns a new Java byte array containing the original uncompressed data.
 * Throws an exception on failure (e.g., corrupted input or invalid format).
 */
JNIEXPORT jbyteArray JNICALL
Java_net_openzl_OpenZLJNI_decompressSerial(JNIEnv *env, jclass clazz, jlong decompressor_ptr,
                                          jbyteArray src, jint src_off, jint src_len) {
    if (decompressor_ptr == 0) {
        throw_openzl_exception(env, "[Error OpenZL JNI] Invalid decompressor");
        return NULL;
    }
    
    openzl_decompressor_t *decompressor = (openzl_decompressor_t *)(uintptr_t)decompressor_ptr;
    
    jbyte *src_data = (*env)->GetByteArrayElements(env, src, NULL);
    if (src_data == NULL) {
        throw_out_of_memory(env);
        return NULL;
    }
    
    ZL_Report size_report = ZL_getDecompressedSize(src_data + src_off, src_len);
    if (ZL_isError(size_report)) {
        (*env)->ReleaseByteArrayElements(env, src, src_data, JNI_ABORT);
        throw_openzl_report_error(env, size_report);
        return NULL;
    }
    
    size_t decompressed_size = ZL_validResult(size_report);
    
    void *decompressed_data = malloc(decompressed_size);
    if (decompressed_data == NULL) {
        (*env)->ReleaseByteArrayElements(env, src, src_data, JNI_ABORT);
        throw_out_of_memory(env);
        return NULL;
    }
    
    ZL_Report decompress_report = ZL_decompress(
        decompressed_data, decompressed_size,
        src_data + src_off, src_len
    );
    
    (*env)->ReleaseByteArrayElements(env, src, src_data, JNI_ABORT);
    
    if (ZL_isError(decompress_report)) {
        free(decompressed_data);
        throw_openzl_report_error(env, decompress_report);
        return NULL;
    }
    
    size_t result_size = ZL_validResult(decompress_report);
    
    jbyteArray result = (*env)->NewByteArray(env, result_size);
    if (result == NULL) {
        free(decompressed_data);
        throw_out_of_memory(env);
        return NULL;
    }
    
    (*env)->SetByteArrayRegion(env, result, 0, result_size, (jbyte *)decompressed_data);
    free(decompressed_data);
    
    return result;
}

/**
 * Decompresses OpenZL-compressed byte data directly into a pre-allocated destination buffer.
 * Returns the actual decompressed size on success, or -1 if an error occurs (exception thrown).
 * The destination buffer must be large enough to hold the full decompressed output.
 */
JNIEXPORT jint JNICALL
Java_net_openzl_OpenZLJNI_decompressSerialToBuffer(JNIEnv *env, jclass clazz, jlong decompressor_ptr,
                                                  jbyteArray src, jint src_off, jint src_len,
                                                  jbyteArray dest, jint dest_off, jint max_dest_len) {
    if (decompressor_ptr == 0) {
        throw_openzl_exception(env, "[Error OpenZL JNI] Invalid decompressor");
        return -1;
    }
    
    jbyte *src_data = (*env)->GetByteArrayElements(env, src, NULL);
    if (src_data == NULL) {
        throw_out_of_memory(env);
        return -1;
    }

    jbyte *dest_data = (*env)->GetByteArrayElements(env, dest, NULL);
    if (dest_data == NULL) {
        (*env)->ReleaseByteArrayElements(env, src, src_data, JNI_ABORT);
        throw_out_of_memory(env);
        return -1;
    }
    
    ZL_Report decompress_report = ZL_decompress(
        dest_data + dest_off, max_dest_len,
        src_data + src_off, src_len
    );
    
    (*env)->ReleaseByteArrayElements(env, src, src_data, JNI_ABORT);
    (*env)->ReleaseByteArrayElements(env, dest, dest_data, 0);
    
    if (ZL_isError(decompress_report)) {
        throw_openzl_exception(env, ZL_ErrorCode_toString(ZL_errorCode(decompress_report)));
        return -1;
    }
    
    size_t decompressed_size = ZL_validResult(decompress_report);
    return (jint)decompressed_size;
}

/**
 * Decompresses OpenZL-compressed numeric data back into a Java int array.
 * Expects the original data to have been compressed as 32-bit signed integers.
 * Returns a new int array with the decompressed values, or throws an exception on failure.
 */
JNIEXPORT jintArray JNICALL
Java_net_openzl_OpenZLJNI_decompressNumericInts(JNIEnv *env, jclass clazz, jlong decompressor_ptr, jbyteArray src) {
    if (decompressor_ptr == 0) {
        throw_openzl_exception(env, "[Error OpenZL JNI] Invalid decompressor");
        return NULL;
    }
    
    openzl_decompressor_t *decompressor = (openzl_decompressor_t *)(uintptr_t)decompressor_ptr;
    
    jsize src_len = (*env)->GetArrayLength(env, src);
    jbyte *src_data = (*env)->GetByteArrayElements(env, src, NULL);
    
    if (src_data == NULL) {
        throw_out_of_memory(env);
        return NULL;
    }
    
    ZL_Report size_report = ZL_getDecompressedSize(src_data, src_len);
    if (ZL_isError(size_report)) {
        (*env)->ReleaseByteArrayElements(env, src, src_data, JNI_ABORT);
        throw_openzl_exception(env, ZL_ErrorCode_toString(ZL_errorCode(size_report)));
        return NULL;
    }
    
    size_t decompressed_size = ZL_validResult(size_report);
    
    void *decompressed_data = malloc(decompressed_size);
    if (decompressed_data == NULL) {
        (*env)->ReleaseByteArrayElements(env, src, src_data, JNI_ABORT);
        throw_out_of_memory(env);
        return NULL;
    }
    
    ZL_OutputInfo outputInfo;
    ZL_Report decompress_report = ZL_DCtx_decompressTyped(
        decompressor->ctx,
        &outputInfo,
        decompressed_data, decompressed_size,
        src_data, src_len
    );
    
    (*env)->ReleaseByteArrayElements(env, src, src_data, JNI_ABORT);
    
    if (ZL_isError(decompress_report)) {
        free(decompressed_data);
        throw_openzl_exception(env, ZL_ErrorCode_toString(ZL_errorCode(decompress_report)));
        return NULL;
    }
    
    jsize array_len = outputInfo.numElts;
    jintArray result = (*env)->NewIntArray(env, array_len);
    if (result == NULL) {
        free(decompressed_data);
        throw_out_of_memory(env);
        return NULL;
    }
    
    (*env)->SetIntArrayRegion(env, result, 0, array_len, (jint *)decompressed_data);
    free(decompressed_data);
    
    return result;
}

/**
 * Decompresses OpenZL-compressed numeric data back into a Java long array.
 * Expects the original data to have been compressed as 64-bit signed integers.
 * Returns a new long array with the decompressed values, or throws an exception on failure.
 */
JNIEXPORT jlongArray JNICALL
Java_net_openzl_OpenZLJNI_decompressNumericLongs(JNIEnv *env, jclass clazz, jlong decompressor_ptr, jbyteArray src) {
    if (decompressor_ptr == 0) {
        throw_openzl_exception(env, "[Error OpenZL JNI] Invalid decompressor");
        return NULL;
    }
    
    openzl_decompressor_t *decompressor = (openzl_decompressor_t *)(uintptr_t)decompressor_ptr;
    
    jsize src_len = (*env)->GetArrayLength(env, src);
    jbyte *src_data = (*env)->GetByteArrayElements(env, src, NULL);
    
    if (src_data == NULL) {
        throw_out_of_memory(env);
        return NULL;
    }
    
    ZL_Report size_report = ZL_getDecompressedSize(src_data, src_len);
    if (ZL_isError(size_report)) {
        (*env)->ReleaseByteArrayElements(env, src, src_data, JNI_ABORT);
        throw_openzl_exception(env, ZL_ErrorCode_toString(ZL_errorCode(size_report)));
        return NULL;
    }
    
    size_t decompressed_size = ZL_validResult(size_report);
    
    void *decompressed_data = malloc(decompressed_size);
    if (decompressed_data == NULL) {
        (*env)->ReleaseByteArrayElements(env, src, src_data, JNI_ABORT);
        throw_out_of_memory(env);
        return NULL;
    }
    
    ZL_OutputInfo outputInfo;
    ZL_Report decompress_report = ZL_DCtx_decompressTyped(
        decompressor->ctx,
        &outputInfo,
        decompressed_data, decompressed_size,
        src_data, src_len
    );
    
    (*env)->ReleaseByteArrayElements(env, src, src_data, JNI_ABORT);
    
    if (ZL_isError(decompress_report)) {
        free(decompressed_data);
        throw_openzl_exception(env, ZL_ErrorCode_toString(ZL_errorCode(decompress_report)));
        return NULL;
    }
    
    jsize array_len = outputInfo.numElts;
    jlongArray result = (*env)->NewLongArray(env, array_len);
    if (result == NULL) {
        free(decompressed_data);
        throw_out_of_memory(env);
        return NULL;
    }
    
    (*env)->SetLongArrayRegion(env, result, 0, array_len, (jlong *)decompressed_data);
    free(decompressed_data);
    
    return result;
}

/**
 * Decompresses OpenZL-compressed numeric data back into a Java float array.
 * Expects the original data to have been compressed as 32-bit floating-point values.
 * Returns a new float array with the decompressed values, or throws an exception on failure.
 */
JNIEXPORT jfloatArray JNICALL
Java_net_openzl_OpenZLJNI_decompressNumericFloats(JNIEnv *env, jclass clazz, jlong decompressor_ptr, jbyteArray src) {
    if (decompressor_ptr == 0) {
        throw_openzl_exception(env, "[Error OpenZL JNI] Invalid decompressor");
        return NULL;
    }
    
    openzl_decompressor_t *decompressor = (openzl_decompressor_t *)(uintptr_t)decompressor_ptr;
    
    jsize src_len = (*env)->GetArrayLength(env, src);
    jbyte *src_data = (*env)->GetByteArrayElements(env, src, NULL);
    
    if (src_data == NULL) {
        throw_out_of_memory(env);
        return NULL;
    }
    
    ZL_Report size_report = ZL_getDecompressedSize(src_data, src_len);
    if (ZL_isError(size_report)) {
        (*env)->ReleaseByteArrayElements(env, src, src_data, JNI_ABORT);
        throw_openzl_exception(env, ZL_ErrorCode_toString(ZL_errorCode(size_report)));
        return NULL;
    }
    
    size_t decompressed_size = ZL_validResult(size_report);
    
    void *decompressed_data = malloc(decompressed_size);
    if (decompressed_data == NULL) {
        (*env)->ReleaseByteArrayElements(env, src, src_data, JNI_ABORT);
        throw_out_of_memory(env);
        return NULL;
    }
    
    ZL_OutputInfo outputInfo;
    ZL_Report decompress_report = ZL_DCtx_decompressTyped(
        decompressor->ctx,
        &outputInfo,
        decompressed_data, decompressed_size,
        src_data, src_len
    );
    
    (*env)->ReleaseByteArrayElements(env, src, src_data, JNI_ABORT);
    
    if (ZL_isError(decompress_report)) {
        free(decompressed_data);
        throw_openzl_exception(env, ZL_ErrorCode_toString(ZL_errorCode(decompress_report)));
        return NULL;
    }
    
    jsize array_len = outputInfo.numElts;
    jfloatArray result = (*env)->NewFloatArray(env, array_len);
    if (result == NULL) {
        free(decompressed_data);
        throw_out_of_memory(env);
        return NULL;
    }
    
    (*env)->SetFloatArrayRegion(env, result, 0, array_len, (jfloat *)decompressed_data);
    free(decompressed_data);
    
    return result;
}

/**
 * Decompresses OpenZL-compressed numeric data back into a Java double array.
 * Expects the original data to have been compressed as 64-bit floating-point values.
 * Returns a new double array with the decompressed values, or throws an exception on failure.
 */
JNIEXPORT jdoubleArray JNICALL
Java_net_openzl_OpenZLJNI_decompressNumericDoubles(JNIEnv *env, jclass clazz, jlong decompressor_ptr, jbyteArray src) {
    if (decompressor_ptr == 0) {
        throw_openzl_exception(env, "[Error OpenZL JNI] Invalid decompressor");
        return NULL;
    }
    
    openzl_decompressor_t *decompressor = (openzl_decompressor_t *)(uintptr_t)decompressor_ptr;
    
    jsize src_len = (*env)->GetArrayLength(env, src);
    jbyte *src_data = (*env)->GetByteArrayElements(env, src, NULL);
    
    if (src_data == NULL) {
        throw_out_of_memory(env);
        return NULL;
    }
    
    ZL_Report size_report = ZL_getDecompressedSize(src_data, src_len);
    if (ZL_isError(size_report)) {
        (*env)->ReleaseByteArrayElements(env, src, src_data, JNI_ABORT);
        throw_openzl_exception(env, ZL_ErrorCode_toString(ZL_errorCode(size_report)));
        return NULL;
    }
    
    size_t decompressed_size = ZL_validResult(size_report);
    
    void *decompressed_data = malloc(decompressed_size);
    if (decompressed_data == NULL) {
        (*env)->ReleaseByteArrayElements(env, src, src_data, JNI_ABORT);
        throw_out_of_memory(env);
        return NULL;
    }
    
    ZL_OutputInfo outputInfo;
    ZL_Report decompress_report = ZL_DCtx_decompressTyped(
        decompressor->ctx,
        &outputInfo,
        decompressed_data, decompressed_size,
        src_data, src_len
    );
    
    (*env)->ReleaseByteArrayElements(env, src, src_data, JNI_ABORT);
    
    if (ZL_isError(decompress_report)) {
        free(decompressed_data);
        throw_openzl_exception(env, ZL_ErrorCode_toString(ZL_errorCode(decompress_report)));
        return NULL;
    }
    
    jsize array_len = outputInfo.numElts;
    jdoubleArray result = (*env)->NewDoubleArray(env, array_len);
    if (result == NULL) {
        free(decompressed_data);
        throw_out_of_memory(env);
        return NULL;
    }
    
    (*env)->SetDoubleArrayRegion(env, result, 0, array_len, (jdouble *)decompressed_data);
    free(decompressed_data);
    
    return result;
}


JNIEXPORT jint JNICALL
Java_net_openzl_OpenZLJNI_compressBound(JNIEnv *env, jclass clazz, jint src_len) {
    return (jint)ZL_compressBound(src_len);
}

/**
 * Analyzes OpenZL-compressed data and returns metadata about its contents.
 * Returns a CompressionInfo object containing decompressed size, compressed size,
 * data type (e.g., SERIAL, NUMERIC), and inferred compression graph.
 * Throws an exception if the input is invalid or metadata cannot be extracted.
 */

JNIEXPORT jobject JNICALL
Java_net_openzl_OpenZLJNI_getCompressionInfo(JNIEnv *env, jclass clazz, jbyteArray compressed_data) {
    if (compressed_data == NULL) {
        throw_exception(env, "java/lang/IllegalArgumentException", "[Error OpenZL JNI] Compressed data cannot be null");
        return NULL;
    }
    
    jsize compressed_len = (*env)->GetArrayLength(env, compressed_data);
    if (compressed_len <= 0) {
        throw_openzl_exception(env, "[Error OpenZL JNI] Compressed data is empty");
        return NULL;
    }
    
    jbyte *compressed_bytes = (*env)->GetByteArrayElements(env, compressed_data, NULL);
    if (compressed_bytes == NULL) {
        throw_out_of_memory(env);
        return NULL;
    }
    
    ZL_Report compressed_size_result = ZL_getCompressedSize(compressed_bytes, compressed_len);
    if (ZL_isError(compressed_size_result)) {
        (*env)->ReleaseByteArrayElements(env, compressed_data, compressed_bytes, JNI_ABORT);
        throw_openzl_exception(env, "[Error OpenZL JNI] Failed to get compressed size");
        return NULL;
    }
    
    ZL_FrameInfo *frame_info = ZL_FrameInfo_create(compressed_bytes, compressed_len);
    if (frame_info == NULL) {
        (*env)->ReleaseByteArrayElements(env, compressed_data, compressed_bytes, JNI_ABORT);
        throw_openzl_exception(env, "[Error OpenZL JNI] Failed to create frame info");
        return NULL;
    }
    
    ZL_Report decompressed_size_result = ZL_FrameInfo_getDecompressedSize(frame_info, 0);
    if (ZL_isError(decompressed_size_result)) {
        ZL_FrameInfo_free(frame_info);
        (*env)->ReleaseByteArrayElements(env, compressed_data, compressed_bytes, JNI_ABORT);
        throw_openzl_exception(env, "[Error OpenZL JNI] Failed to get decompressed size");
        return NULL;
    }
    
    ZL_Report output_type_result = ZL_FrameInfo_getOutputType(frame_info, 0);
    if (ZL_isError(output_type_result)) {
        ZL_FrameInfo_free(frame_info);
        (*env)->ReleaseByteArrayElements(env, compressed_data, compressed_bytes, JNI_ABORT);
        throw_openzl_exception(env, "[Error OpenZL JNI] Failed to get output type");
        return NULL;
    }
    
    size_t compressed_size = ZL_validResult(compressed_size_result);
    size_t decompressed_size = ZL_validResult(decompressed_size_result);
    int output_type = (int)ZL_validResult(output_type_result);
    
    const char *data_type_str;
    switch (output_type) {
        case 0:
            data_type_str = "SERIAL";
            break;
        case 2:
            data_type_str = "NUMERIC";
            break;
        case 1:
            data_type_str = "STRUCT";
            break;
        case 3:
            data_type_str = "STRING";
            break;
        default:
            data_type_str = "UNKNOWN";
            break;
    }
    
    ZL_FrameInfo_free(frame_info);
    (*env)->ReleaseByteArrayElements(env, compressed_data, compressed_bytes, JNI_ABORT);
    
    jclass compression_info_class = (*env)->FindClass(env, "net/openzl/CompressionInfo");
    if (compression_info_class == NULL) {
        throw_openzl_exception(env, "[Error OpenZL JNI] Failed to find CompressionInfo class");
        return NULL;
    }
    
    jmethodID constructor = (*env)->GetMethodID(env, compression_info_class, "<init>", "(JJLnet/openzl/CompressionGraph;Ljava/lang/String;)V");
    if (constructor == NULL) {
        throw_openzl_exception(env, "[Error OpenZL JNI] Failed to find CompressionInfo constructor");
        return NULL;
    }
    
    jclass compression_graph_class = (*env)->FindClass(env, "net/openzl/CompressionGraph");
    if (compression_graph_class == NULL) {
        throw_openzl_exception(env, "[Error OpenZL JNI] Failed to find CompressionGraph class");
        return NULL;
    }
    
    jmethodID from_id_method = (*env)->GetStaticMethodID(env, compression_graph_class, "fromId", "(I)Lnet/openzl/CompressionGraph;");
    if (from_id_method == NULL) {
        throw_openzl_exception(env, "[Error OpenZL JNI] Failed to find CompressionGraph.fromId method");
        return NULL;
    }
    
    int graph_id;
    switch (output_type) {
        case 2:
            graph_id = 1;
            break;
        case 0:
            graph_id = 0;
            break;
        case 1:
        case 3:
        default:
            graph_id = 0;
            break;
    }
    
    jobject compression_graph = (*env)->CallStaticObjectMethod(env, compression_graph_class, from_id_method, graph_id);
    if (compression_graph == NULL) {
        throw_openzl_exception(env, "[Error OpenZL JNI] Failed to create CompressionGraph object");
        return NULL;
    }
    
    jstring data_type_string = (*env)->NewStringUTF(env, data_type_str);
    if (data_type_string == NULL) {
        throw_out_of_memory(env);
        return NULL;
    }
    
    jobject compression_info = (*env)->NewObject(env, compression_info_class, constructor,
                                                  (jlong)decompressed_size,
                                                  (jlong)compressed_size,
                                                  compression_graph,
                                                  data_type_string);
    
    return compression_info;
}