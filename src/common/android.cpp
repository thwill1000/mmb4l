/*-*****************************************************************************

MMBasic for Linux (MMB4L)

mmb4a.c

Copyright 2021-2025 Geoff Graham, Peter Mather and Thomas Hugo Williams.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holders nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

4. The name MMBasic be used when referring to the interpreter in any
   documentation and promotional material and the original copyright message
   be displayed  on the console at startup (additional copyright messages may
   be added).

5. All advertising materials mentioning features or use of this software must
   display the following acknowledgement: This product includes software
   developed by Geoff Graham, Peter Mather and Thomas Hugo Williams.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*******************************************************************************/

#include <SDL.h>

#include "android.h"
#include "saf_bridge.h"

extern "C" {
#include "logger.h"
} // extern "C"

void android_init(void) {
    initialize_saf_system();
}

void android_term(void) {
    cleanup_saf_system();    
}

void android_show_soft_keyboard(void) {
#if defined(__ANDROID__)
    LOG_INFO("Starting text input...");
    SDL_StartTextInput();
    LOG_INFO("Text input active: %d", SDL_IsTextInputActive());            
#endif
}

#if 0

#include <jni.h>
#include <SDL_thread.h>
#include <android/log.h>
#include <string.h>
#include <stdlib.h>


void request_documents_access();
int has_mmbasic_folder();
void show_mmbasic_folder_missing();

SDL_sem* completionSemaphore;

void android_init(void) {
#if defined(__ANDROID__)
    LOG_INFO("Internal storage path: %s", android_path());
    completionSemaphore = SDL_CreateSemaphore(0);
    // TODO: What if this fails.
    if (!has_documents_access()) {
        request_documents_access();
        SDL_SemWait(completionSemaphore);
        SDL_DestroySemaphore(completionSemaphore);
        if (!has_mmbasic_folder()) {
            LOG_INFO("Create new semaphore");
            completionSemaphore = SDL_CreateSemaphore(0);
            LOG_INFO("Show MMBasic folder missing dialog");
            show_mmbasic_folder_missing();
            SDL_SemWait(completionSemaphore);
        }
        LOG_INFO("Wallaby");
    }
#endif
}

const char *android_path(void) {
#if defined(__ANDROID__)
   return SDL_AndroidGetInternalStoragePath();
#else
   return "/"; // TODO
#endif
}

// #include <jni.h>
// #include <SDL.h>
// #include <android/log.h>
// #include <string.h>
// #include <stdlib.h>

// #define LOG_TAG "DocumentsAccess"
// #define LOG_INFO(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
// #define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// Global state
static int documents_access_granted = 0;
static int documents_access_requested = 0;

// Function to request documents access from native code
void request_documents_access() {
    LOG_INFO("Requesting document access");
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    jobject activity = (jobject)SDL_AndroidGetActivity();
    
    if (!env || !activity) {
        LOG_ERROR("Failed to get JNI environment or activity");
        return;
    }
    
    jclass activityClass = (*env)->GetObjectClass(env, activity);
    jmethodID requestMethod = (*env)->GetStaticMethodID(env, activityClass, 
                                                       "requestDocumentsAccess", "()V");
    
    if (requestMethod) {
        (*env)->CallStaticVoidMethod(env, activityClass, requestMethod);
        documents_access_requested = 1;
        LOG_INFO("Documents access requested");
    } else {
        LOG_ERROR("Failed to find requestDocumentsAccess method");
    }
    
    (*env)->DeleteLocalRef(env, activityClass);
}

// Function to check if we have documents access
int has_documents_access() {
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    jobject activity = (jobject)SDL_AndroidGetActivity();
    
    if (!env || !activity) {
        return 0;
    }
    
    jclass activityClass = (*env)->GetObjectClass(env, activity);
    jmethodID hasAccessMethod = (*env)->GetStaticMethodID(env, activityClass, 
                                                         "hasDocumentsAccess", "()Z");
    
    jboolean result = JNI_FALSE;
    if (hasAccessMethod) {
        result = (*env)->CallStaticBooleanMethod(env, activityClass, hasAccessMethod);
    }
    
    (*env)->DeleteLocalRef(env, activityClass);
    return result == JNI_TRUE ? 1 : 0;
}

// Function to list files in Documents directory
char** list_documents_files(int* count) {
    *count = 0;
    
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    jobject activity = (jobject)SDL_AndroidGetActivity();
    
    if (!env || !activity) {
        return NULL;
    }
    
    jclass activityClass = (*env)->GetObjectClass(env, activity);
    jmethodID listMethod = (*env)->GetStaticMethodID(env, activityClass, 
                                                    "listDocumentsFiles", "()[Ljava/lang/String;");
    
    if (!listMethod) {
        (*env)->DeleteLocalRef(env, activityClass);
        return NULL;
    }
    
    jobjectArray fileArray = (jobjectArray)(*env)->CallStaticObjectMethod(env, activityClass, listMethod);
    
    if (!fileArray) {
        (*env)->DeleteLocalRef(env, activityClass);
        return NULL;
    }
    
    jsize arrayLength = (*env)->GetArrayLength(env, fileArray);
    *count = arrayLength;
    
    if (arrayLength == 0) {
        (*env)->DeleteLocalRef(env, fileArray);
        (*env)->DeleteLocalRef(env, activityClass);
        return NULL;
    }
    
    char** fileNames = malloc(arrayLength * sizeof(char*));
    
    for (int i = 0; i < arrayLength; i++) {
        jstring fileName = (jstring)(*env)->GetObjectArrayElement(env, fileArray, i);
        const char* fileNameStr = (*env)->GetStringUTFChars(env, fileName, NULL);
        
        fileNames[i] = malloc(strlen(fileNameStr) + 1);
        strcpy(fileNames[i], fileNameStr);
        
        (*env)->ReleaseStringUTFChars(env, fileName, fileNameStr);
        (*env)->DeleteLocalRef(env, fileName);
    }
    
    (*env)->DeleteLocalRef(env, fileArray);
    (*env)->DeleteLocalRef(env, activityClass);
    
    return fileNames;
}

// Function to read a file from Documents directory
unsigned char* read_documents_file(const char* fileName, size_t* fileSize) {
    *fileSize = 0;
    
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    jobject activity = (jobject)SDL_AndroidGetActivity();
    
    if (!env || !activity) {
        return NULL;
    }
    
    jclass activityClass = (*env)->GetObjectClass(env, activity);
    jmethodID readMethod = (*env)->GetStaticMethodID(env, activityClass, 
                                                    "readDocumentFile", "(Ljava/lang/String;)[B");
    
    if (!readMethod) {
        (*env)->DeleteLocalRef(env, activityClass);
        return NULL;
    }
    
    jstring jFileName = (*env)->NewStringUTF(env, fileName);
    jbyteArray byteArray = (jbyteArray)(*env)->CallStaticObjectMethod(env, activityClass, 
                                                                      readMethod, jFileName);
    
    if (!byteArray) {
        (*env)->DeleteLocalRef(env, jFileName);
        (*env)->DeleteLocalRef(env, activityClass);
        return NULL;
    }
    
    jsize arrayLength = (*env)->GetArrayLength(env, byteArray);
    *fileSize = arrayLength;
    
    unsigned char* fileData = malloc(arrayLength);
    (*env)->GetByteArrayRegion(env, byteArray, 0, arrayLength, (jbyte*)fileData);
    
    (*env)->DeleteLocalRef(env, byteArray);
    (*env)->DeleteLocalRef(env, jFileName);
    (*env)->DeleteLocalRef(env, activityClass);
    
    return fileData;
}

// Free file list memory
void free_file_list(char** fileNames, int count) {
    if (fileNames) {
        for (int i = 0; i < count; i++) {
            free(fileNames[i]);
        }
        free(fileNames);
    }
}

// JNI callbacks from Java
JNIEXPORT void JNICALL 
Java_com_sockpuppetstudios_mmb4a_MainActivity_nativeOnDocumentsAccessGranted(JNIEnv* env, jobject obj) {
    documents_access_granted = 1;
    LOG_INFO("Documents access granted");
    SDL_SemPost(completionSemaphore);
    
    // You can add your own callback here or set a flag that your main loop checks
}

JNIEXPORT void JNICALL 
Java_com_sockpuppetstudios_mmb4a_MainActivity_nativeOnDocumentsAccessDenied(JNIEnv* env, jobject obj) {
    documents_access_granted = 0;
    LOG_INFO("Documents access denied");
    SDL_SemPost(completionSemaphore);
    
    // Handle denial - maybe show a message to the user
}

// Function to ensure permissions are saved before app exit
void ensure_permissions_persisted() {
    LOG_INFO("Persist permissions");
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    jobject activity = (jobject)SDL_AndroidGetActivity();
    
    if (!env || !activity) {
        return;
    }
    
    jclass activityClass = (*env)->GetObjectClass(env, activity);
    jmethodID ensureMethod = (*env)->GetStaticMethodID(env, activityClass, 
                                                      "ensurePermissionsPersisted", "()V");
    
    if (ensureMethod) {
        (*env)->CallStaticVoidMethod(env, activityClass, ensureMethod);
        LOG_INFO("Permissions persistence ensured");
        
        // Give Android time to save
        SDL_Delay(200);
    } else {
        LOG_ERROR("Failed to find ensurePermissionsPersisted method");
    }
    
    (*env)->DeleteLocalRef(env, activityClass);
}

// Example usage function you can call from your main SDL loop
void example_documents_usage() {
    if (!has_documents_access()) {
        LOG_INFO("No documents access, requesting...");
        request_documents_access();
        return;
    }
    
    // List files
    int fileCount;
    char** files = list_documents_files(&fileCount);
    
    LOG_INFO("Found %d files in Documents:", fileCount);
    for (int i = 0; i < fileCount; i++) {
        LOG_INFO("  - %s", files[i]);
        
        // Try to read the first file as an example
        if (i == 0) {
            size_t fileSize;
            unsigned char* fileData = read_documents_file(files[i], &fileSize);
            if (fileData) {
                LOG_INFO("Read %zu bytes from %s", fileSize, files[i]);
                // Process your file data here
                free(fileData);
            }
        }
    }
    
    free_file_list(files, fileCount);
}

// Function to check if mmbasic folder exists
int has_mmbasic_folder() {
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    jobject activity = (jobject)SDL_AndroidGetActivity();
    
    if (!env || !activity) {
        return 0;
    }
    
    jclass activityClass = (*env)->GetObjectClass(env, activity);
    jmethodID hasFolderMethod = (*env)->GetStaticMethodID(env, activityClass, 
                                                         "hasMmbasicFolder", "()Z");
    
    jboolean result = JNI_FALSE;
    if (hasFolderMethod) {
        result = (*env)->CallStaticBooleanMethod(env, activityClass, hasFolderMethod);
    }
    
    (*env)->DeleteLocalRef(env, activityClass);
    return result == JNI_TRUE ? 1 : 0;
}

// Function to show mmbasic folder missing dialog
void show_mmbasic_folder_missing() {
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    jobject activity = (jobject)SDL_AndroidGetActivity();
    
    if (!env || !activity) {
        return;
    }
    
    jclass activityClass = (*env)->GetObjectClass(env, activity);
    jmethodID showMethod = (*env)->GetStaticMethodID(env, activityClass, 
                                                    "showMmbasicFolderMissing", "()V");
    
    if (showMethod) {
        (*env)->CallStaticVoidMethod(env, activityClass, showMethod);
    } else {
        LOG_ERROR("Failed to find showMmbasicFolderMissing method");
    }
    
    (*env)->DeleteLocalRef(env, activityClass);
}

#endif
