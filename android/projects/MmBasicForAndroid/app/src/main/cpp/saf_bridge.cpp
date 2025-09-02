/*
 * saf_bridge.cpp - Complete Storage Access Framework Bridge for SDL2/NDK
 *
 * This file provides a complete interface between C++ native code and Android's
 * Storage Access Framework (SAF) for accessing files in Documents/mmbasic directory.
 *
 * Features:
 * - Full file I/O operations (read, write, delete, list)
 * - Directory operations (create, delete, list)
 * - Streaming I/O with C stdio interface (fopen, fread, fwrite, fseek, etc.)
 * - Proper Activity lifecycle management
 * - Persistent directory permissions
 */

#include <jni.h>
#include <string>
#include <vector>
#include <android/log.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "SDL.h"

#include "saf_bridge.h"

// #define LOG_TAG "MMBasic_SAF"
#define LOG_TAG "MMB4A"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// Global state
static JavaVM* g_jvm = nullptr;
static jclass g_mainActivityClass = nullptr;
static jobject g_activityInstance = nullptr;
static bool g_directoryReady = false;

// Structure to track open file handles for streaming I/O
struct SAFFileHandle {
    int fdId;        // Java-side file descriptor ID
    int nativeFd;    // Native file descriptor
    FILE* file;      // C stdio FILE pointer
    std::string filename;
    std::string mode;
};

static std::vector<SAFFileHandle*> g_openFiles;
static int g_nextHandle = 1;

/*
 * JNI callback functions - called from Java MainActivity
 */

extern "C" JNIEXPORT void JNICALL
Java_com_sockpuppetstudios_mmb4a_MainActivity_nativeOnDirectoryReady(JNIEnv* env, jclass clazz) {
    g_directoryReady = true;
    LOGD("Directory access ready");
}

extern "C" JNIEXPORT void JNICALL
Java_com_sockpuppetstudios_mmb4a_MainActivity_nativeOnActivityPause(JNIEnv* env, jclass clazz) {
    LOGD("Activity paused - SAF operations may be suspended");
}

extern "C" JNIEXPORT void JNICALL
Java_com_sockpuppetstudios_mmb4a_MainActivity_nativeOnActivityResume(JNIEnv* env, jclass clazz) {
    LOGD("Activity resumed - SAF operations available");
}

extern "C" JNIEXPORT void JNICALL
Java_com_sockpuppetstudios_mmb4a_MainActivity_nativeOnActivityDestroy(JNIEnv* env, jclass clazz) {
    LOGD("Activity destroyed - cleaning up SAF bridge");
    // Cleanup will be called from the main cleanup function
}

/*
 * Core SAF Bridge Functions
 */

// Initialize the SAF bridge with proper Activity context
bool saf_init() {
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    if (!env) {
        LOGE("Failed to get JNI environment");
        return false;
    }

    // Get the Activity instance from SDL
    jobject activity = (jobject)SDL_AndroidGetActivity();
    if (!activity) {
        LOGE("Failed to get SDL Activity");
        return false;
    }

    // Create global reference to the activity
    g_activityInstance = env->NewGlobalRef(activity);
    if (!g_activityInstance) {
        LOGE("Failed to create global reference to Activity");
        return false;
    }

    // Get the Activity's class
    jclass localClass = env->GetObjectClass(activity);
    if (!localClass) {
        LOGE("Failed to get Activity class");
        return false;
    }

    g_mainActivityClass = (jclass)env->NewGlobalRef(localClass);
    env->DeleteLocalRef(localClass);

    if (!g_mainActivityClass) {
        LOGE("Failed to create global reference to Activity class");
        return false;
    }

    LOGD("SAF bridge initialized successfully");
    return true;
}

// Request directory access from user
void saf_request_directory_access() {
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    if (!env || !g_activityInstance) {
        LOGE("Environment or Activity not available");
        return;
    }

    // Call the static method to request directory access
    jmethodID method = env->GetStaticMethodID(g_mainActivityClass, "requestDirectoryAccess", "()V");
    if (method) {
        env->CallStaticVoidMethod(g_mainActivityClass, method);
    } else {
        LOGE("Could not find requestDirectoryAccess method");
    }

    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
}

// Check if directory access is available
bool saf_has_directory_access() {
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    if (!env) {
        return false;
    }

    jmethodID method = env->GetStaticMethodID(g_mainActivityClass, "hasDirectoryAccess", "()Z");
    if (method) {
        return env->CallStaticBooleanMethod(g_mainActivityClass, method);
    }
    return false;
}

// Utility function to check if Activity is available
bool saf_is_activity_available() {
    return g_activityInstance != nullptr && SDL_AndroidGetActivity() != nullptr;
}

/*
 * File Operations
 */

// List files in the mmbasic directory
std::vector<std::string> saf_list_files(std::string dirname) {
    std::vector<std::string> files;

    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    if (!env) {
        return files;
    }

    jmethodID method = env->GetStaticMethodID(g_mainActivityClass, "listFiles", "(Ljava/lang/String;)[Ljava/lang/String;");
    if (!method) return files;

    jstring jdirname = env->NewStringUTF(dirname.c_str());
    jobjectArray jfileArray = (jobjectArray)env->CallStaticObjectMethod(g_mainActivityClass, method, jdirname);
    if (!jfileArray) return files;

    //LOGD("foobar");

    int count = env->GetArrayLength(jfileArray);
    for (int i = 0; i < count; i++) {
        jstring jfilename = (jstring)env->GetObjectArrayElement(jfileArray, i);
        const char* filename = env->GetStringUTFChars(jfilename, nullptr);
        //LOGD("%s", filename);
        files.push_back(std::string(filename));
        //LOGD("Calling ReleaseStringUTFChars");
        env->ReleaseStringUTFChars(jfilename, filename);
        //LOGD("Calling DeleteLocalRef(jfilename)");
        env->DeleteLocalRef(jfilename);
    }

    //LOGD("Calling DeleteLocalRef(jfileArray)");
    env->DeleteLocalRef(jdirname);
    env->DeleteLocalRef(jfileArray);
    //LOGD("Returning");
    return files;
}

// Read file from mmbasic directory
std::vector<uint8_t> saf_read_file(const std::string& filename) {
    std::vector<uint8_t> data;

    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    if (!env) {
        return data;
    }

    jmethodID method = env->GetStaticMethodID(g_mainActivityClass, "readFile", "(Ljava/lang/String;)[B");
    if (!method) return data;

    jstring jfilename = env->NewStringUTF(filename.c_str());
    jbyteArray jdata = (jbyteArray)env->CallStaticObjectMethod(g_mainActivityClass, method, jfilename);

    if (jdata) {
        int length = env->GetArrayLength(jdata);
        data.resize(length);
        env->GetByteArrayRegion(jdata, 0, length, (jbyte*)data.data());
        env->DeleteLocalRef(jdata);
    }

    env->DeleteLocalRef(jfilename);
    return data;
}

// Write file to mmbasic directory
bool saf_write_file(const std::string& filename, const std::vector<uint8_t>& data) {
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    if (!env) {
        return false;
    }

    jmethodID method = env->GetStaticMethodID(g_mainActivityClass, "writeFile", "(Ljava/lang/String;[B)Z");
    if (!method) return false;

    jstring jfilename = env->NewStringUTF(filename.c_str());
    jbyteArray jdata = env->NewByteArray(data.size());
    env->SetByteArrayRegion(jdata, 0, data.size(), (const jbyte*)data.data());

    bool result = env->CallStaticBooleanMethod(g_mainActivityClass, method, jfilename, jdata);

    env->DeleteLocalRef(jfilename);
    env->DeleteLocalRef(jdata);
    return result;
}

// Delete file from mmbasic directory
bool saf_delete_file(const std::string& filename) {
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    if (!env) {
        return false;
    }

    jmethodID method = env->GetStaticMethodID(g_mainActivityClass, "safDeleteFile", "(Ljava/lang/String;)Z");
    if (!method) return false;

    jstring jfilename = env->NewStringUTF(filename.c_str());
    bool result = env->CallStaticBooleanMethod(g_mainActivityClass, method, jfilename);
    env->DeleteLocalRef(jfilename);

    return result;
}

// Check if file exists
bool saf_file_exists(const std::string& filename) {
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    if (!env) {
        return false;
    }

    jmethodID method = env->GetStaticMethodID(g_mainActivityClass, "fileExists", "(Ljava/lang/String;)Z");
    if (!method) return false;

    jstring jfilename = env->NewStringUTF(filename.c_str());
    bool result = env->CallStaticBooleanMethod(g_mainActivityClass, method, jfilename);
    env->DeleteLocalRef(jfilename);

    return result;
}

// Get file size
long saf_get_file_size(const std::string& filename) {
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    if (!env) {
        return -1;
    }

    jmethodID method = env->GetStaticMethodID(g_mainActivityClass, "getFileSize", "(Ljava/lang/String;)J");
    if (!method) return -1;

    jstring jfilename = env->NewStringUTF(filename.c_str());
    long result = env->CallStaticLongMethod(g_mainActivityClass, method, jfilename);
    env->DeleteLocalRef(jfilename);

    return result;
}

/*
 * Directory Operations
 */

// Create subdirectory
bool saf_create_directory(const std::string& dirname) {
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    if (!env) {
        return false;
    }

    jmethodID method = env->GetStaticMethodID(g_mainActivityClass, "createDirectory", "(Ljava/lang/String;)Z");
    if (!method) return false;

    jstring jdirname = env->NewStringUTF(dirname.c_str());
    bool result = env->CallStaticBooleanMethod(g_mainActivityClass, method, jdirname);
    env->DeleteLocalRef(jdirname);

    return result;
}

// Delete subdirectory (must be empty)
bool saf_delete_directory(const std::string& dirname) {
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    if (!env) {
        return false;
    }

    jmethodID method = env->GetStaticMethodID(g_mainActivityClass, "deleteDirectory", "(Ljava/lang/String;)Z");
    if (!method) return false;

    jstring jdirname = env->NewStringUTF(dirname.c_str());
    bool result = env->CallStaticBooleanMethod(g_mainActivityClass, method, jdirname);
    env->DeleteLocalRef(jdirname);

    return result;
}

// Check if directory exists
bool saf_directory_exists(const std::string& dirname) {
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    if (!env) {
        return false;
    }

    jmethodID method = env->GetStaticMethodID(g_mainActivityClass, "directoryExists", "(Ljava/lang/String;)Z");
    if (!method) return false;

    jstring jdirname = env->NewStringUTF(dirname.c_str());
    bool result = env->CallStaticBooleanMethod(g_mainActivityClass, method, jdirname);
    env->DeleteLocalRef(jdirname);

    return result;
}

// List subdirectories
std::vector<std::string> saf_list_directories() {
    std::vector<std::string> dirs;

    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    if (!env) {
        return dirs;
    }

    jmethodID method = env->GetStaticMethodID(g_mainActivityClass, "listDirectories", "()[Ljava/lang/String;");
    if (!method) return dirs;

    jobjectArray jdirArray = (jobjectArray)env->CallStaticObjectMethod(g_mainActivityClass, method);
    if (!jdirArray) return dirs;

    int count = env->GetArrayLength(jdirArray);
    for (int i = 0; i < count; i++) {
        jstring jdirname = (jstring)env->GetObjectArrayElement(jdirArray, i);
        const char* dirname = env->GetStringUTFChars(jdirname, nullptr);
        dirs.push_back(std::string(dirname));
        env->ReleaseStringUTFChars(jdirname, dirname);
        env->DeleteLocalRef(jdirname);
    }

    env->DeleteLocalRef(jdirArray);
    return dirs;
}

/*
 * Streaming I/O Functions - C stdio interface for large files
 */

// Open a file for streaming I/O
int saf_fopen(const std::string& filename, const std::string& mode) {
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    if (!env) {
        return -1;
    }

    // Call Java method to open file and get FD ID
    jmethodID method = env->GetStaticMethodID(g_mainActivityClass, "openFileForStreaming",
                                             "(Ljava/lang/String;Ljava/lang/String;)I");
    if (!method) return -1;

    jstring jfilename = env->NewStringUTF(filename.c_str());
    jstring jmode = env->NewStringUTF(mode.c_str());
    int fdId = env->CallStaticIntMethod(g_mainActivityClass, method, jfilename, jmode);

    env->DeleteLocalRef(jfilename);
    env->DeleteLocalRef(jmode);

    if (fdId < 0) {
        return -1;
    }

    // Get the native file descriptor
    method = env->GetStaticMethodID(g_mainActivityClass, "getNativeFileDescriptor", "(I)I");
    if (!method) return -1;

    int nativeFd = env->CallStaticIntMethod(g_mainActivityClass, method, fdId);
    if (nativeFd < 0) {
        // Close the Java FD if we can't get native FD
        method = env->GetStaticMethodID(g_mainActivityClass, "closeFileDescriptor", "(I)Z");
        if (method) {
            env->CallStaticBooleanMethod(g_mainActivityClass, method, fdId);
        }
        return -1;
    }

    // Duplicate the file descriptor so we own it
    int dupFd = dup(nativeFd);
    if (dupFd < 0) {
        method = env->GetStaticMethodID(g_mainActivityClass, "closeFileDescriptor", "(I)Z");
        if (method) {
            env->CallStaticBooleanMethod(g_mainActivityClass, method, fdId);
        }
        return -1;
    }

    // Create FILE* from the duplicated file descriptor
    FILE* file = fdopen(dupFd, mode.c_str());
    if (!file) {
        close(dupFd);
        method = env->GetStaticMethodID(g_mainActivityClass, "closeFileDescriptor", "(I)Z");
        if (method) {
            env->CallStaticBooleanMethod(g_mainActivityClass, method, fdId);
        }
        return -1;
    }

    // Create and store file handle
    SAFFileHandle* handle = new SAFFileHandle;
    handle->fdId = fdId;
    handle->nativeFd = dupFd;
    handle->file = file;
    handle->filename = filename;
    handle->mode = mode;

    int handleId = g_nextHandle++;

    // Store in our tracking vector (resize if needed)
    if (g_openFiles.size() <= handleId) {
        g_openFiles.resize(handleId + 1, nullptr);
    }
    g_openFiles[handleId] = handle;

    return handleId;
}

// Get FILE* pointer from handle ID
FILE* saf_get_file_pointer(int handleId) {
    if (handleId <= 0 || handleId >= g_openFiles.size() || !g_openFiles[handleId]) {
        return nullptr;
    }
    return g_openFiles[handleId]->file;
}

// Close a file handle
bool saf_fclose(int handleId) {
    if (handleId <= 0 || handleId >= g_openFiles.size() || !g_openFiles[handleId]) {
        return false;
    }

    SAFFileHandle* handle = g_openFiles[handleId];

    // Close the FILE*
    if (handle->file) {
        fclose(handle->file);  // This also closes the duplicated FD
    }

    // Close the Java-side file descriptor
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    if (env) {
        jmethodID method = env->GetStaticMethodID(g_mainActivityClass, "closeFileDescriptor", "(I)Z");
        if (method) {
            env->CallStaticBooleanMethod(g_mainActivityClass, method, handle->fdId);
        }
    }

    delete handle;
    g_openFiles[handleId] = nullptr;

    return true;
}

// Convenience functions using the FILE* directly
size_t saf_fread(void* ptr, size_t size, size_t count, int handleId) {
    FILE* file = saf_get_file_pointer(handleId);
    if (!file) return 0;
    return fread(ptr, size, count, file);
}

size_t saf_fwrite(const void* ptr, size_t size, size_t count, int handleId) {
    FILE* file = saf_get_file_pointer(handleId);
    if (!file) return 0;
    return fwrite(ptr, size, count, file);
}

int saf_fseek(int handleId, long offset, int whence) {
    FILE* file = saf_get_file_pointer(handleId);
    if (!file) return -1;
    return fseek(file, offset, whence);
}

long saf_ftell(int handleId) {
    FILE* file = saf_get_file_pointer(handleId);
    if (!file) return -1;
    return ftell(file);
}

int saf_fflush(int handleId) {
    FILE* file = saf_get_file_pointer(handleId);
    if (!file) return EOF;
    return fflush(file);
}

int saf_feof(int handleId) {
    FILE* file = saf_get_file_pointer(handleId);
    if (!file) return 1;
    return feof(file);
}

int saf_ferror(int handleId) {
    FILE* file = saf_get_file_pointer(handleId);
    if (!file) return 1;
    return ferror(file);
}

bool saf_rename_file(const std::string& old_path, const std::string& new_path) {
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    if (!env) {
        return false;
    }

    jmethodID method = env->GetStaticMethodID(g_mainActivityClass, "renameFile", "(Ljava/lang/String;Ljava/lang/String;)Z");
    if (!method) return false;

    jstring jOldPath = env->NewStringUTF(old_path.c_str());
    jstring jNewPath = env->NewStringUTF(new_path.c_str());

    bool result = env->CallStaticBooleanMethod(g_mainActivityClass, method, jOldPath, jNewPath);

    env->DeleteLocalRef(jOldPath);
    env->DeleteLocalRef(jNewPath);

    return result;
}

void saf_rewind(int handleId) {
    FILE* file = saf_get_file_pointer(handleId);
    if (file) {
        rewind(file);
    }
}

/*
 * Lifecycle and Cleanup Functions
 */

// Clean up all open files (call at shutdown)
void saf_cleanup_all_files() {
    for (size_t i = 0; i < g_openFiles.size(); i++) {
        if (g_openFiles[i]) {
            saf_fclose(i);
        }
    }
    g_openFiles.clear();
}

// Clean up when Activity is destroyed
void saf_cleanup() {
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    if (env) {
        if (g_activityInstance) {
            env->DeleteGlobalRef(g_activityInstance);
            g_activityInstance = nullptr;
        }
        if (g_mainActivityClass) {
            env->DeleteGlobalRef(g_mainActivityClass);
            g_mainActivityClass = nullptr;
        }
    }
    g_directoryReady = false;
    LOGD("SAF bridge cleaned up");
}

/*
 * High-level initialization and management functions
 */

// Call this in your main SDL loop initialization
void initialize_saf_system() {
    if (!saf_init()) {
        LOGE("Failed to initialize SAF system");
        // Handle error - maybe fall back to internal storage
        return;
    }

    // Check if we already have directory access
    if (!saf_has_directory_access()) {
        LOGD("No directory access - will need to request from user");
        // You can request immediately or wait for user action
        saf_request_directory_access();
    } else {
        LOGD("Directory access already granted");
        g_directoryReady = true;
    }
}

// Call this when your app shuts down
void cleanup_saf_system() {
    saf_cleanup_all_files();
    saf_cleanup();
}

/*
 * Utility functions for common operations
 */

// Read a text file as a string
std::string saf_read_text_file(const std::string& filename) {
    std::vector<uint8_t> data = saf_read_file(filename);
    if (data.empty()) {
        return "";
    }
    return std::string(data.begin(), data.end());
}

// Write a text file from a string
bool saf_write_text_file(const std::string& filename, const std::string& content) {
    std::vector<uint8_t> data(content.begin(), content.end());
    return saf_write_file(filename, data);
}

// Convenience function to check if SAF system is ready for file operations
bool saf_is_ready() {
    return saf_is_activity_available() && saf_has_directory_access();
}

/**
 * Get comprehensive file/directory statistics using the getFileInfo() Java method
 * This version uses the alternative Java method that takes an output array
 */
SAFFileInfo saf_get_file_info(const std::string& filename) {
    LOGD("saf_get_file_info(%s)", filename.c_str());
    SAFFileInfo info = {};  // Initialize all fields to 0/false

    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    if (!env) {
        LOGE("saf_get_file_info: No JNI environment");
        return info;
    }

    if (!g_mainActivityClass) {
        LOGE("saf_get_file_info: No activity class");
        return info;
    }

    // Create a long array to receive the info
    jlongArray joutInfo = env->NewLongArray(6);
    if (!joutInfo) {
        LOGE("saf_get_file_info: Failed to create output array");
        return info;
    }

    // Call Java method
    jmethodID method = env->GetStaticMethodID(g_mainActivityClass, "getFileInfo",
                                             "(Ljava/lang/String;[J)Z");
    if (!method) {
        LOGE("saf_get_file_info: Method getFileInfo not found");
        env->DeleteLocalRef(joutInfo);
        return info;
    }

    jstring jfilename = env->NewStringUTF(filename.c_str());
    if (!jfilename) {
        LOGE("saf_get_file_info: Failed to create Java string");
        env->DeleteLocalRef(joutInfo);
        return info;
    }

    jboolean result = env->CallStaticBooleanMethod(g_mainActivityClass, method, jfilename, joutInfo);
    env->DeleteLocalRef(jfilename);

    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        LOGE("saf_get_file_info: Java exception occurred");
        env->DeleteLocalRef(joutInfo);
        return info;
    }

    if (!result) {
        LOGD("saf_get_file_info: File not found or error: %s", filename.c_str());
        env->DeleteLocalRef(joutInfo);
        return info;  // exists = false
    }

    // Get the info from the array
    jlong* infoData = env->GetLongArrayElements(joutInfo, nullptr);
    if (infoData) {
        info.exists = (infoData[0] != 0);
        info.is_file = (infoData[1] != 0);
        info.is_directory = (infoData[2] != 0);
        info.size = infoData[3];
        info.last_modified = infoData[4];
        info.can_read = (infoData[5] & 1) != 0;
        info.can_write = (infoData[5] & 2) != 0;

        env->ReleaseLongArrayElements(joutInfo, infoData, JNI_ABORT);

        LOGD("saf_get_file_info: %s - exists:%d, file:%d, dir:%d, size:%ld, modified:%ld, read:%d, write:%d",
             filename.c_str(), info.exists, info.is_file, info.is_directory,
             info.size, info.last_modified, info.can_read, info.can_write);
    } else {
        LOGE("saf_get_file_info: Failed to get info array elements");
    }

    env->DeleteLocalRef(joutInfo);
    return info;
}

/**
 * Convenience function to check if a path is a directory
 */
bool saf_is_directory(const std::string& path) {
    SAFFileInfo info = saf_get_file_info(path);
    return info.exists && info.is_directory;
}

/**
 * Convenience function to check if a path is a file
 */
bool saf_is_file(const std::string& path) {
    SAFFileInfo info = saf_get_file_info(path);
    return info.exists && info.is_file;
}

long saf_free_space() {
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    if (!env) {
        LOGE("Failed to get JNI environment");
        return -1;
    }

    jmethodID method = env->GetStaticMethodID(g_mainActivityClass, "getFreeSpace", "()J");
    if (!method) {
        LOGE("Failed to get method: MainActivity#getFreeSpace");
        return -1;
    }

    return env->CallStaticLongMethod(g_mainActivityClass, method);
}
