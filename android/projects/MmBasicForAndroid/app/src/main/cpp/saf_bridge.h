/*
 * saf_bridge.h - Storage Access Framework Bridge for SDL2/NDK
 * 
 * This header provides a C++ interface to Android's Storage Access Framework (SAF)
 * for accessing files in the Documents/mmbasic directory. It bridges between native
 * C++ code and Java SAF APIs through JNI.
 * 
 * Features:
 * - Full file I/O operations with both simple and streaming interfaces
 * - Directory management operations
 * - Proper Android Activity lifecycle integration
 * - Persistent directory permissions
 * - C stdio-compatible streaming I/O for large files
 * 
 * Usage:
 *   1. Call initialize_saf_system() during app startup
 *   2. Request directory access with saf_request_directory_access() if needed
 *   3. Use saf_* functions for file operations
 *   4. Call cleanup_saf_system() during app shutdown
 */

#ifndef SAF_BRIDGE_H
#define SAF_BRIDGE_H

#include <string>
#include <vector>
#include <cstdint>
#include <cstdio>

// #ifdef __cplusplus
// extern "C" {
// #endif

/*
 * System Initialization and Management
 */

/**
 * Initialize the SAF bridge system.
 * Must be called after SDL initialization and before any SAF operations.
 * 
 * @return true if initialization successful, false otherwise
 */
bool saf_init();

/**
 * High-level SAF system initialization.
 * Calls saf_init() and checks for existing directory permissions.
 * Call this in your main initialization code.
 */
void initialize_saf_system();

/**
 * Clean up SAF system resources.
 * Closes all open files and releases Activity references.
 * Call this during app shutdown.
 */
void cleanup_saf_system();

/**
 * Check if the SAF system is ready for file operations.
 * 
 * @return true if Activity is available and directory access is granted
 */
bool saf_is_ready();

/**
 * Check if Activity context is available.
 * 
 * @return true if SDL Activity is accessible
 */
bool saf_is_activity_available();

/*
 * Directory Access Management
 */

/**
 * Request directory access from the user.
 * Shows the Android directory picker dialog.
 * The result is delivered asynchronously via nativeOnDirectoryReady().
 */
void saf_request_directory_access();

/**
 * Check if directory access permission is granted.
 * 
 * @return true if we have persistent access to Documents/mmbasic
 */
bool saf_has_directory_access();

/*
 * Basic File Operations
 */

/**
 * Read an entire file into memory.
 * 
 * @param filename Name of file in Documents/mmbasic directory
 * @return Vector containing file data, empty if file doesn't exist or error
 */
std::vector<uint8_t> saf_read_file(const std::string& filename);

/**
 * Write data to a file.
 * Creates the file if it doesn't exist, overwrites if it does.
 * 
 * @param filename Name of file in Documents/mmbasic directory
 * @param data Vector containing data to write
 * @return true if write successful, false otherwise
 */
bool saf_write_file(const std::string& filename, const std::vector<uint8_t>& data);

/**
 * Delete a file.
 * 
 * @param filename Name of file in Documents/mmbasic directory
 * @return true if deletion successful, false if file doesn't exist or error
 */
bool saf_delete_file(const std::string& filename);

/**
 * Check if a file exists.
 * 
 * @param filename Name of file in Documents/mmbasic directory
 * @return true if file exists and is readable
 */
bool saf_file_exists(const std::string& filename);

/**
 * Get the size of a file in bytes.
 * 
 * @param filename Name of file in Documents/mmbasic directory
 * @return File size in bytes, or -1 if file doesn't exist or error
 */
long saf_get_file_size(const std::string& filename);

/**
 * List all files in the Documents/mmbasic directory.
 * 
 * @return Vector of filenames (without path)
 */
std::vector<std::string> saf_list_files();

/*
 * Text File Convenience Functions
 */

/**
 * Read a text file as a string.
 * 
 * @param filename Name of text file in Documents/mmbasic directory
 * @return File contents as string, empty if file doesn't exist or error
 */
std::string saf_read_text_file(const std::string& filename);

/**
 * Write a string to a text file.
 * 
 * @param filename Name of text file in Documents/mmbasic directory
 * @param content String content to write
 * @return true if write successful, false otherwise
 */
bool saf_write_text_file(const std::string& filename, const std::string& content);

/*
 * Directory Operations
 */

/**
 * Create a subdirectory within Documents/mmbasic.
 * 
 * @param dirname Name of directory to create
 * @return true if creation successful or directory already exists, false otherwise
 */
bool saf_create_directory(const std::string& dirname);

/**
 * Delete a subdirectory.
 * The directory must be empty to be deleted successfully.
 * 
 * @param dirname Name of directory to delete
 * @return true if deletion successful, false if directory doesn't exist, 
 *         is not empty, or error occurred
 */
bool saf_delete_directory(const std::string& dirname);

/**
 * Check if a subdirectory exists.
 * 
 * @param dirname Name of directory to check
 * @return true if directory exists
 */
bool saf_directory_exists(const std::string& dirname);

/**
 * List all subdirectories in Documents/mmbasic.
 * 
 * @return Vector of directory names (without path)
 */
std::vector<std::string> saf_list_directories();

/*
 * Streaming I/O Operations - C stdio interface
 * 
 * These functions provide a familiar C stdio interface for efficient
 * reading/writing of large files with seeking support.
 */

/**
 * Open a file for streaming I/O.
 * 
 * @param filename Name of file in Documents/mmbasic directory
 * @param mode File access mode:
 *             "r", "rb" - Read only
 *             "w", "wb" - Write only (truncates file)
 *             "a", "ab" - Append only
 *             "r+", "rb+", "r+b" - Read/write
 *             "w+", "wb+", "w+b" - Read/write (truncates file)
 * @return File handle ID (>= 0) on success, -1 on error
 */
int saf_fopen(const std::string& filename, const std::string& mode);

/**
 * Close a file opened with saf_fopen().
 * 
 * @param handleId File handle ID returned by saf_fopen()
 * @return true if close successful, false otherwise
 */
bool saf_fclose(int handleId);

/**
 * Read data from a file.
 * 
 * @param ptr Pointer to buffer to read data into
 * @param size Size of each element to read
 * @param count Number of elements to read
 * @param handleId File handle ID returned by saf_fopen()
 * @return Number of elements actually read
 */
size_t saf_fread(void* ptr, size_t size, size_t count, int handleId);

/**
 * Write data to a file.
 * 
 * @param ptr Pointer to data to write
 * @param size Size of each element to write
 * @param count Number of elements to write
 * @param handleId File handle ID returned by saf_fopen()
 * @return Number of elements actually written
 */
size_t saf_fwrite(const void* ptr, size_t size, size_t count, int handleId);

/**
 * Seek to a position in the file.
 * 
 * @param handleId File handle ID returned by saf_fopen()
 * @param offset Byte offset to seek to
 * @param whence Seek origin: SEEK_SET (beginning), SEEK_CUR (current), SEEK_END (end)
 * @return 0 on success, -1 on error
 */
int saf_fseek(int handleId, long offset, int whence);

/**
 * Get current file position.
 * 
 * @param handleId File handle ID returned by saf_fopen()
 * @return Current file position in bytes, -1 on error
 */
long saf_ftell(int handleId);

/**
 * Flush file buffers to storage.
 * 
 * @param handleId File handle ID returned by saf_fopen()
 * @return 0 on success, EOF on error
 */
int saf_fflush(int handleId);

/**
 * Check if end-of-file has been reached.
 * 
 * @param handleId File handle ID returned by saf_fopen()
 * @return Non-zero if EOF reached, 0 otherwise
 */
int saf_feof(int handleId);

/**
 * Check if a file error has occurred.
 * 
 * @param handleId File handle ID returned by saf_fopen()
 * @return Non-zero if error occurred, 0 otherwise
 */
int saf_ferror(int handleId);

/**
 * Reset file position to beginning.
 * 
 * @param handleId File handle ID returned by saf_fopen()
 */
void saf_rewind(int handleId);

/**
 * Get the underlying FILE* pointer for a handle.
 * Advanced usage - allows direct use of stdio functions.
 * 
 * @param handleId File handle ID returned by saf_fopen()
 * @return FILE* pointer, or nullptr if handle is invalid
 */
FILE* saf_get_file_pointer(int handleId);

/*
 * Internal Cleanup Functions
 */

/**
 * Clean up all open file handles.
 * Called automatically by cleanup_saf_system().
 * You normally don't need to call this directly.
 */
void saf_cleanup_all_files();

/**
 * Clean up SAF bridge resources.
 * Called automatically by cleanup_saf_system().
 * You normally don't need to call this directly.
 */
void saf_cleanup();

/*
 * JNI Callback Functions
 * 
 * These are called from the Java MainActivity and should not be
 * called directly from C++ code.
 */

// #ifdef __cplusplus
// } // extern "C"

/*
 * C++ Convenience Functions
 * 
 * These functions provide additional C++-specific functionality
 * using STL containers and strings.
 */

/**
 * Example usage class demonstrating common operations.
 */
// class SAFFileManager {
// public:
//     /**
//      * Initialize the SAF system and request directory access if needed.
//      * 
//      * @param requestAccessIfNeeded If true, automatically request directory access
//      * @return true if system is ready for file operations
//      */
//     static bool initialize(bool requestAccessIfNeeded = true);
    
//     /**
//      * Check if the system is ready for file operations.
//      */
//     static bool isReady() { return saf_is_ready(); }
    
//     /**
//      * Load a configuration file as key-value pairs.
//      * Each line should be in format "key=value".
//      * 
//      * @param filename Configuration file name
//      * @return Map of configuration key-value pairs
//      */
//     static std::map<std::string, std::string> loadConfig(const std::string& filename);
    
//     /**
//      * Save configuration as key-value pairs.
//      * 
//      * @param filename Configuration file name
//      * @param config Map of configuration key-value pairs
//      * @return true if save successful
//      */
//     static bool saveConfig(const std::string& filename, 
//                           const std::map<std::string, std::string>& config);
    
//     /**
//      * Copy a file within the Documents/mmbasic directory.
//      * 
//      * @param srcFilename Source file name
//      * @param destFilename Destination file name
//      * @return true if copy successful
//      */
//     static bool copyFile(const std::string& srcFilename, const std::string& destFilename);
    
//     /**
//      * Get file information including size and existence.
//      */
//     struct FileInfo {
//         bool exists;
//         long size;
//         std::string name;
//     };
    
//     /**
//      * Get information about a file.
//      * 
//      * @param filename File name to check
//      * @return FileInfo structure with file details
//      */
//     static FileInfo getFileInfo(const std::string& filename);
    
//     /**
//      * Get a list of all files with their information.
//      * 
//      * @return Vector of FileInfo structures for all files
//      */
//     static std::vector<FileInfo> getAllFiles();
// };

// #endif // __cplusplus

/*
 * Error Codes and Constants
 */

// File handle constants
#define SAF_INVALID_HANDLE  (-1)

// Common file modes
#define SAF_MODE_READ       "rb"
#define SAF_MODE_WRITE      "wb"
#define SAF_MODE_APPEND     "ab"
#define SAF_MODE_READ_WRITE "rb+"
#define SAF_MODE_WRITE_READ "wb+"

// Maximum filename length
#define SAF_MAX_FILENAME_LENGTH 255

// Maximum path length for subdirectories
#define SAF_MAX_PATH_LENGTH 1024

#endif // SAF_BRIDGE_H

/*
 * Usage Examples:
 * 
 * // Basic initialization
 * initialize_saf_system();
 * if (!saf_has_directory_access()) {
 *     saf_request_directory_access();
 * }
 * 
 * // Simple file operations
 * std::string content = saf_read_text_file("config.txt");
 * saf_write_text_file("log.txt", "Application started");
 * 
 * // Binary file operations  
 * std::vector<uint8_t> data = {0x48, 0x65, 0x6C, 0x6C, 0x6F};
 * saf_write_file("data.bin", data);
 * 
 * // Streaming I/O for large files
 * int handle = saf_fopen("largefile.dat", SAF_MODE_READ);
 * if (handle != SAF_INVALID_HANDLE) {
 *     char buffer[1024];
 *     while (!saf_feof(handle)) {
 *         size_t bytesRead = saf_fread(buffer, 1, sizeof(buffer), handle);
 *         // Process data...
 *     }
 *     saf_fclose(handle);
 * }
 * 
 * // Directory operations
 * saf_create_directory("projects");
 * auto files = saf_list_files();
 * auto dirs = saf_list_directories();
 * 
 * // Cleanup
 * cleanup_saf_system();
 */
