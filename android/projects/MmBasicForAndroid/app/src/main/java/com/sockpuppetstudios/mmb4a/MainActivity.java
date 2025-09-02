/*
 * MainActivity.java - Complete SAF Bridge Android Activity for SDL2/NDK
 *
 * This Activity extends SDLActivity and provides the Java-side implementation
 * of Storage Access Framework (SAF) operations for MMBasic Android app.
 *
 * Features:
 * - Persistent directory access to Documents/mmbasic
 * - Complete file and directory operations via SAF
 * - File descriptor access for streaming I/O
 * - Proper lifecycle management
 * - Error handling and logging
 */

package com.sockpuppetstudios.mmb4a;

import android.app.Activity;
import android.content.Intent;
import android.content.SharedPreferences;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.os.ParcelFileDescriptor;
import android.os.StatFs;
import android.provider.DocumentsContract;
import android.content.ContentResolver;
import android.database.Cursor;
import android.util.Log;

import androidx.documentfile.provider.DocumentFile;

import org.libsdl.app.SDLActivity;

import java.io.File;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class MainActivity extends SDLActivity {
    private static final String TAG = "MMB4A";

    // Request codes for Activity results
    private static final int REQUEST_CODE_OPEN_DIRECTORY = 1001;
    private static final int REQUEST_CODE_OPEN_FILE = 1002;
    private static final int REQUEST_CODE_CREATE_FILE = 1003;

    // Preferences key for storing directory URI
    private static final String PREFS_NAME = "mmbasic_prefs";
    private static final String KEY_DIRECTORY_URI = "mmbasic_directory_uri";

    // Static instance for native code access
    private static MainActivity instance;

    // SAF directory access
    private Uri mmbasicDirectoryUri = null;
    private DocumentFile mmbasicDirectory = null;

    // File descriptor management for streaming I/O
    private static Map<Integer, ParcelFileDescriptor> openFileDescriptors = new HashMap<>();
    private static int nextFdId = 1;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Log.d(TAG, "onCreate called");
        super.onCreate(savedInstanceState);
        instance = this;

        // Try to restore previously granted directory access
        restoreDirectoryAccess();

        Log.d(TAG, "MainActivity created, directory access: " + hasDirectoryAccess());
    }

    @Override
    protected void onResume() {
        super.onResume();
        Log.d(TAG, "onResume called");
        nativeOnActivityResume();
    }

    @Override
    protected void onPause() {
        super.onPause();
        Log.d(TAG, "onPause called");
        nativeOnActivityPause();
    }

    @Override
    protected void onDestroy() {
        Log.d(TAG, "onDestroy called");
        closeAllFileDescriptors();
        nativeOnActivityDestroy();
        instance = null;
        super.onDestroy();
    }

    /*
     * Directory Access Management
     */

    /**
     * Restore directory access from saved preferences
     */
    private void restoreDirectoryAccess() {
        SharedPreferences prefs = getSharedPreferences(PREFS_NAME, MODE_PRIVATE);
        String savedUriString = prefs.getString(KEY_DIRECTORY_URI, null);

        if (savedUriString != null) {
            try {
                mmbasicDirectoryUri = Uri.parse(savedUriString);
                mmbasicDirectory = DocumentFile.fromTreeUri(this, mmbasicDirectoryUri);

                // Verify the directory still exists and we have permissions
                if (mmbasicDirectory == null || !mmbasicDirectory.exists() || !mmbasicDirectory.canRead()) {
                    Log.w(TAG, "Saved directory URI is no longer valid");
                    clearDirectoryAccess();
                } else {
                    Log.d(TAG, "Successfully restored directory access");
                }
            } catch (Exception e) {
                Log.e(TAG, "Error restoring directory access", e);
                clearDirectoryAccess();
            }
        }
    }

    /**
     * Clear directory access and remove from preferences
     */
    private void clearDirectoryAccess() {
        mmbasicDirectoryUri = null;
        mmbasicDirectory = null;

        SharedPreferences prefs = getSharedPreferences(PREFS_NAME, MODE_PRIVATE);
        prefs.edit().remove(KEY_DIRECTORY_URI).apply();
    }

    /**
     * Called from native code to request directory access
     */
    public static void requestDirectoryAccess() {
        Log.d(TAG, "requestDirectoryAccess called from native");
        if (instance != null) {
            instance.runOnUiThread(() -> instance.openDirectoryPicker());
        } else {
            Log.e(TAG, "MainActivity instance is null");
        }
    }

    /**
     * Show the system directory picker
     */
    private void openDirectoryPicker() {
        Log.d(TAG, "Opening directory picker");
        Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT_TREE);

        // Try to start in Documents directory
        try {
            Uri documentsUri = DocumentsContract.buildDocumentUri(
                "com.android.externalstorage.documents",
                "primary:Documents"
            );
            intent.putExtra(DocumentsContract.EXTRA_INITIAL_URI, documentsUri);
        } catch (Exception e) {
            Log.w(TAG, "Could not set initial directory", e);
        }

        try {
            startActivityForResult(intent, REQUEST_CODE_OPEN_DIRECTORY);
        } catch (Exception e) {
            Log.e(TAG, "Failed to start directory picker", e);
        }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        Log.d(TAG, "onActivityResult: requestCode=" + requestCode + ", resultCode=" + resultCode);

        if (requestCode == REQUEST_CODE_OPEN_DIRECTORY && resultCode == RESULT_OK) {
            if (data != null && data.getData() != null) {
                handleDirectoryAccessGranted(data.getData());
            } else {
                Log.e(TAG, "Directory picker returned null data");
            }
        }
    }

    /**
     * Handle directory access being granted by user
     */
    private void handleDirectoryAccessGranted(Uri treeUri) {
        Log.d(TAG, "Directory access granted: " + treeUri.toString());

        try {
            // Take persistent permission
            getContentResolver().takePersistableUriPermission(
                treeUri,
                Intent.FLAG_GRANT_READ_URI_PERMISSION | Intent.FLAG_GRANT_WRITE_URI_PERMISSION
            );

            mmbasicDirectoryUri = treeUri;
            mmbasicDirectory = DocumentFile.fromTreeUri(this, treeUri);

            if (mmbasicDirectory != null) {
                // Create mmbasic subdirectory if it doesn't exist
                DocumentFile mmbasicSubDir = mmbasicDirectory.findFile("mmbasic");
                if (mmbasicSubDir == null || !mmbasicSubDir.isDirectory()) {
                    Log.d(TAG, "Creating mmbasic subdirectory");
                    mmbasicSubDir = mmbasicDirectory.createDirectory("mmbasic");
                }

                if (mmbasicSubDir != null && mmbasicSubDir.exists()) {
                    mmbasicDirectory = mmbasicSubDir;

                    // Save URI for future use
                    SharedPreferences prefs = getSharedPreferences(PREFS_NAME, MODE_PRIVATE);
                    prefs.edit().putString(KEY_DIRECTORY_URI, treeUri.toString()).apply();

                    Log.d(TAG, "MMBasic directory ready");

                    // Notify native code
                    nativeOnDirectoryReady();
                } else {
                    Log.e(TAG, "Failed to create or access mmbasic subdirectory");
                    clearDirectoryAccess();
                }
            } else {
                Log.e(TAG, "Failed to create DocumentFile from tree URI");
                clearDirectoryAccess();
            }

        } catch (SecurityException e) {
            Log.e(TAG, "Failed to take persistent permission", e);
            clearDirectoryAccess();
        } catch (Exception e) {
            Log.e(TAG, "Error handling directory access", e);
            clearDirectoryAccess();
        }
    }

    /*
     * Native Method Declarations
     */

    public static native void nativeOnDirectoryReady();
    public static native void nativeOnActivityPause();
    public static native void nativeOnActivityResume();
    public static native void nativeOnActivityDestroy();

    /*
     * Directory and File Status Functions
     */

    /**
     * Called from native code to check if directory access is available
     */
    public static boolean hasDirectoryAccess() {
        boolean hasAccess = instance != null &&
                           instance.mmbasicDirectory != null &&
                           instance.mmbasicDirectory.exists() &&
                           instance.mmbasicDirectory.canRead();

        Log.v(TAG, "hasDirectoryAccess: " + hasAccess);
        return hasAccess;
    }

    /*
     * File Operations
     */

    /**
     * Called from native code to list files in the directory
     */
    public static String[] listFiles(String dirname) {
        Log.v(TAG, "listFiles: " + dirname);
        if (!hasDirectoryAccess()) {
            Log.w(TAG, "listFiles: No directory access");
            return new String[0];
        }

        if (!dirname.startsWith("/")) {
            Log.e(TAG, "dirName does not start with '/'");
            return new String[0];
        }

        dirname = dirname.trim();

        try {
            DocumentFile targetDirectory = instance.mmbasicDirectory;

            // Navigate to subdirectory if specified
            if (!dirname.isEmpty()) {
                // Split path by '/' and navigate step by step
                String[] pathParts = dirname.split("/");

                for (String part : pathParts) {
                    if (!part.isEmpty()) {
                        DocumentFile nextDir = targetDirectory.findFile(part);
                        if (nextDir == null || !nextDir.exists() || !nextDir.isDirectory()) {
                            Log.w(TAG, "listFiles: Subdirectory not found: " + part + " in path: " + dirname);
                            return new String[0];
                        }
                        targetDirectory = nextDir;
                        Log.v(TAG, "listFiles: Navigated to: " + part);
                    }
                }
            }

            DocumentFile[] files = targetDirectory.listFiles();
            if (files == null) {
                Log.w(TAG, "listFiles: listFiles() returned null for path: " + dirname);
                return new String[0];
            }

            List<String> fileNames = new ArrayList<>();

            for (DocumentFile file : files) {
                if (file != null && file.getName() != null) {
                    // Include both files AND directories
                    if (file.isFile()) {
                        fileNames.add(file.getName());
                        Log.v(TAG, "Found file: " + file.getName());
                    } else if (file.isDirectory()) {
                        fileNames.add(file.getName()); //  + "/");  // Optional: add trailing slash for directories
                        Log.v(TAG, "Found directory: " + file.getName());
                    }
                }
            }

            Log.d(TAG, "listFiles: Found " + fileNames.size() + " items in path: " + dirname);
            return fileNames.toArray(new String[0]);

        } catch (Exception e) {
            Log.e(TAG, "Error listing files", e);
            return new String[0];
        }
    }

    /**
     * Called from native code to read a file
     */
    public static byte[] readFile(String filename) {
        Log.v(TAG, "readFile: " + filename);
        if (!hasDirectoryAccess()) {
            Log.w(TAG, "readFile: No directory access");
            return null;
        }

        try {
            DocumentFile file = instance.mmbasicDirectory.findFile(filename);
            if (file == null || !file.exists() || !file.canRead()) {
                Log.w(TAG, "readFile: File not found or not readable: " + filename);
                return null;
            }

            InputStream inputStream = instance.getContentResolver().openInputStream(file.getUri());
            if (inputStream == null) {
                Log.e(TAG, "readFile: Failed to open input stream for: " + filename);
                return null;
            }

            try {
                byte[] buffer = new byte[(int) file.length()];
                int totalBytesRead = 0;
                int bytesRead;

                while (totalBytesRead < buffer.length &&
                       (bytesRead = inputStream.read(buffer, totalBytesRead,
                                                   buffer.length - totalBytesRead)) != -1) {
                    totalBytesRead += bytesRead;
                }

                if (totalBytesRead != buffer.length) {
                    Log.w(TAG, "readFile: Expected " + buffer.length + " bytes, got " + totalBytesRead);
                    byte[] actualBuffer = new byte[totalBytesRead];
                    System.arraycopy(buffer, 0, actualBuffer, 0, totalBytesRead);
                    return actualBuffer;
                }

                Log.d(TAG, "readFile: Successfully read " + totalBytesRead + " bytes from " + filename);
                return buffer;

            } finally {
                inputStream.close();
            }

        } catch (IOException e) {
            Log.e(TAG, "IO error reading file: " + filename, e);
            return null;
        } catch (Exception e) {
            Log.e(TAG, "Error reading file: " + filename, e);
            return null;
        }
    }

    /**
     * Called from native code to write a file
     */
    public static boolean writeFile(String filename, byte[] data) {
        Log.v(TAG, "writeFile: " + filename + " (" + data.length + " bytes)");
        if (!hasDirectoryAccess()) {
            Log.w(TAG, "writeFile: No directory access");
            return false;
        }

        try {
            // Check if file exists, create if not
            DocumentFile file = instance.mmbasicDirectory.findFile(filename);
            if (file == null) {
                file = instance.mmbasicDirectory.createFile("application/octet-stream", filename);
                Log.d(TAG, "writeFile: Created new file: " + filename);
            }

            if (file == null || !file.canWrite()) {
                Log.e(TAG, "writeFile: Cannot write to file: " + filename);
                return false;
            }

            OutputStream outputStream = instance.getContentResolver().openOutputStream(file.getUri(), "wt");
            if (outputStream == null) {
                Log.e(TAG, "writeFile: Failed to open output stream for: " + filename);
                return false;
            }

            try {
                outputStream.write(data);
                outputStream.flush();
                Log.d(TAG, "writeFile: Successfully wrote " + data.length + " bytes to " + filename);
                return true;

            } finally {
                outputStream.close();
            }

        } catch (IOException e) {
            Log.e(TAG, "IO error writing file: " + filename, e);
            return false;
        } catch (Exception e) {
            Log.e(TAG, "Error writing file: " + filename, e);
            return false;
        }
    }

    /**
     * Called from native code to delete a file
     *
     * We cannot call it deleteFile() because android.content.Context which
     * this class extends already includes a method of that name.
     */
    public static boolean safDeleteFile(String path) {
        final String LOG_PREFIX = "MainActivity#safDeleteFile: ";
        Log.d(TAG, LOG_PREFIX + "path=" + path);
        if (!hasDirectoryAccess()) {
            Log.w(TAG, LOG_PREFIX + "No directory access");
            return false;
        }

        try {
            DocumentFile file = findFileByPath(path);
            if (file != null && file.exists()) {
                boolean result = file.delete();
                Log.d(TAG, LOG_PREFIX + " result=" + result);
                return result;
            } else {
                Log.w(TAG, LOG_PREFIX + "File not found: " + path);
                return false;
            }
        } catch (Exception e) {
            Log.e(TAG, LOG_PREFIX + "Error deleting file: " + path, e);
            return false;
        }
    }

    /**
     * Called from native code to check if a file exists
     */
    public static boolean fileExists(String filename) {
        Log.v(TAG, "fileExists: " + filename);
        if (!hasDirectoryAccess()) {
            return false;
        }

        try {
            DocumentFile file = instance.mmbasicDirectory.findFile(filename);
            boolean exists = file != null && file.exists();
            Log.v(TAG, "fileExists: " + filename + " = " + exists);
            return exists;
        } catch (Exception e) {
            Log.e(TAG, "Error checking file existence: " + filename, e);
            return false;
        }
    }

    /**
     * Called from native code to get file size
     */
    public static long getFileSize(String filename) {
        Log.v(TAG, "getFileSize: " + filename);
        if (!hasDirectoryAccess()) {
            return -1;
        }

        try {
            DocumentFile file = instance.mmbasicDirectory.findFile(filename);
            if (file != null && file.exists() && file.isFile()) {
                long size = file.length();
                Log.v(TAG, "getFileSize: " + filename + " = " + size + " bytes");
                return size;
            }
        } catch (Exception e) {
            Log.e(TAG, "Error getting file size: " + filename, e);
        }

        Log.v(TAG, "getFileSize: " + filename + " = -1 (not found)");
        return -1;
    }

    /*
     * Directory Operations
     */

    /**
     * Called from native code to create a subdirectory
     */
    public static boolean createDirectory(String path) {
        Log.d(TAG, "MainActivity#createDirectory() - path=" + path);
        if (!hasDirectoryAccess()) {
            Log.w(TAG, "createDirectory: no directory access");
            return false;
        }

        try {
            DocumentFile dir = findFileByPath(path);
            if (dir != null && dir.exists()) {
                boolean isDir = dir.isDirectory();
                Log.d(TAG, "createDirectory: " + path + " already exists, isDirectory: " + isDir);
                return isDir; // Return true if already exists as directory
            }

            String parentPath = getParentPath(path);
            String dirname = getFilename(path);

            Log.d(TAG, "createDirectory: parentPath=" + parentPath + ", dirname=" + dirname);

            dir = findFileByPath(parentPath);
            if (dir == null || !dir.exists() || !dir.isDirectory()) {
                Log.e(TAG, "createDirectory: " + parentPath + " not found or is not a directory");
                return false;
            }

            DocumentFile newDir = dir.createDirectory(dirname);
            boolean success = newDir != null;
            Log.d(TAG, "createDirectory: result=" + success);
            return success;

        } catch (Exception e) {
            Log.e(TAG, "Error creating directory: " + path, e);
            return false;
        }
    }

    /**
     * Called from native code to delete a directory (must be empty)
     */
    public static boolean deleteDirectory(String dirname) {
        Log.v(TAG, "deleteDirectory: " + dirname);
        if (!hasDirectoryAccess()) {
            Log.w(TAG, "deleteDirectory: No directory access");
            return false;
        }

        try {
            DocumentFile dir = findFileByPath(dirname);
            if (dir != null && dir.exists() && dir.isDirectory()) {
                boolean result = dir.delete();
                Log.d(TAG, "deleteDirectory: " + dirname + " result: " + result);
                return result;
            } else {
                Log.w(TAG, "deleteDirectory: Directory not found: " + dirname);
                return false;
            }
        } catch (Exception e) {
            Log.e(TAG, "Error deleting directory: " + dirname, e);
            return false;
        }
    }

    /**
     * Called from native code to check if a directory exists
     */
    public static boolean directoryExists(String dirname) {
        Log.v(TAG, "directoryExists: " + dirname);
        if (!hasDirectoryAccess()) {
            return false;
        }

        try {
            DocumentFile dir = instance.mmbasicDirectory.findFile(dirname);
            boolean exists = dir != null && dir.exists() && dir.isDirectory();
            Log.v(TAG, "directoryExists: " + dirname + " = " + exists);
            return exists;
        } catch (Exception e) {
            Log.e(TAG, "Error checking directory existence: " + dirname, e);
            return false;
        }
    }

    /**
     * Called from native code to list subdirectories
     */
    public static String[] listDirectories() {
        Log.v(TAG, "listDirectories called");
        if (!hasDirectoryAccess()) {
            Log.w(TAG, "listDirectories: No directory access");
            return new String[0];
        }

        try {
            DocumentFile[] files = instance.mmbasicDirectory.listFiles();
            List<String> dirNames = new ArrayList<>();

            for (DocumentFile file : files) {
                if (file.isDirectory() && file.getName() != null) {
                    dirNames.add(file.getName());
                }
            }

            Log.d(TAG, "listDirectories: Found " + dirNames.size() + " directories");
            return dirNames.toArray(new String[0]);

        } catch (Exception e) {
            Log.e(TAG, "Error listing directories", e);
            return new String[0];
        }
    }

    /*
     * Streaming I/O Support - File Descriptor Management
     */

    /**
     * Open a file and return a native file descriptor ID
     */
    public static int openFileForStreaming(String path, String mode) {
        final String LOG_PREFIX = "MainActivity#safDeleteFile: ";
        Log.v(TAG, LOG_PREFIX + " path=" + path + ", mode=" + mode);
        if (!hasDirectoryAccess()) {
            Log.w(TAG, LOG_PREFIX + " No directory access");
            return -1;
        }

        try {
            DocumentFile file = findFileByPath(path);

            // Check that the file is not a pre-existing directory
            if (file != null && file.exists() && file.isDirectory()) {
                Log.w(TAG, LOG_PREFIX + " Existing file is a directory");
                return -1;
            }

            // Check if file exists or needs to be created
            if (mode.contains("w") || mode.contains("a")) {
                // Writing mode - create file if it doesn't exist
                if (file == null || !file.exists()) {
                    String parentPath = getParentPath(path);
                    String filename = getFilename(path);
                    DocumentFile dir = findFileByPath(parentPath);
                    if (dir == null || !dir.exists() || !dir.isDirectory()) {
                        Log.e(TAG, LOG_PREFIX + parentPath + " not found or is not a directory");
                        return -1;
                    }

                    file = dir.createFile("application/octet-stream", filename);
                    Log.d(TAG, LOG_PREFIX + "Created new file: " + filename);
                }
            } else {
                // Reading mode - file must exist
            }

            if (file == null || !file.exists()) {
                Log.w(TAG, LOG_PREFIX + " File not found: " + path);
                return -1;
            }

            // Determine the ParcelFileDescriptor mode
            String pfdMode;
            if (mode.equals("r") || mode.equals("rb")) {
                pfdMode = "r";
            } else if (mode.equals("w") || mode.equals("wb")) {
                pfdMode = "wt";  // Truncate
            } else if (mode.equals("a") || mode.equals("ab")) {
                pfdMode = "wa";  // Append
            } else if (mode.equals("r+") || mode.equals("rb+") || mode.equals("r+b")) {
                pfdMode = "rw";
            } else if (mode.equals("w+") || mode.equals("wb+") || mode.equals("w+b")) {
                pfdMode = "rwt"; // Read/write with truncate
            } else {
                Log.e(TAG, LOG_PREFIX + " Unsupported mode: " + mode);
                return -1;
            }

            ParcelFileDescriptor pfd = instance.getContentResolver()
                .openFileDescriptor(file.getUri(), pfdMode);

            if (pfd != null) {
                int fdId = nextFdId++;
                openFileDescriptors.put(fdId, pfd);
                Log.d(TAG, LOG_PREFIX + "Opened " + path + " with FD ID: " + fdId);
                return fdId;
            } else {
                Log.e(TAG, LOG_PREFIX + "Failed to get ParcelFileDescriptor for: " + path);
            }

        } catch (FileNotFoundException e) {
            Log.e(TAG, LOG_PREFIX + "File not found for streaming: " + path, e);
        } catch (Exception e) {
            Log.e(TAG, LOG_PREFIX + "Error opening file for streaming: " + path, e);
        }

        return -1;
    }

    static String getParentPath(String path) throws IOException {
        int lastSlashIndex = path.lastIndexOf('/');
        String parentPath = null;
        if (lastSlashIndex == 0) {
            parentPath = "/";
        } else if (lastSlashIndex > 0) {
            parentPath = path.substring(0, lastSlashIndex);
        } else {
            throw new IOException("Malformed absolute path: " + path);
        }
        return parentPath;
    }

    static String getFilename(String path) throws IOException {
        int lastSlashIndex = path.lastIndexOf('/');
        String filename = null;
        if (lastSlashIndex == 0) {
            filename = path.substring(1);
        } else if (lastSlashIndex > 0) {
            filename = path.substring(lastSlashIndex + 1);
        } else {
            throw new IOException("Malformed absolute path: " + path);
        }
        return filename;
    }

    /**
     * Get the actual native file descriptor from our ID
     */
    public static int getNativeFileDescriptor(int fdId) {
        Log.v(TAG, "getNativeFileDescriptor: " + fdId);
        ParcelFileDescriptor pfd = openFileDescriptors.get(fdId);
        if (pfd != null) {
            int nativeFd = pfd.getFd();
            Log.v(TAG, "getNativeFileDescriptor: " + fdId + " -> " + nativeFd);
            return nativeFd;
        }
        Log.w(TAG, "getNativeFileDescriptor: FD ID not found: " + fdId);
        return -1;
    }

    /**
     * Close a file descriptor
     */
    public static boolean closeFileDescriptor(int fdId) {
        Log.v(TAG, "closeFileDescriptor: " + fdId);
        ParcelFileDescriptor pfd = openFileDescriptors.remove(fdId);
        if (pfd != null) {
            try {
                pfd.close();
                Log.d(TAG, "closeFileDescriptor: Closed FD ID: " + fdId);
                return true;
            } catch (IOException e) {
                Log.e(TAG, "Error closing file descriptor: " + fdId, e);
            }
        } else {
            Log.w(TAG, "closeFileDescriptor: FD ID not found: " + fdId);
        }
        return false;
    }

    /**
     * Clean up all open file descriptors (call this in onDestroy)
     */
    private void closeAllFileDescriptors() {
        Log.d(TAG, "closeAllFileDescriptors: Closing " + openFileDescriptors.size() + " file descriptors");
        for (Map.Entry<Integer, ParcelFileDescriptor> entry : openFileDescriptors.entrySet()) {
            try {
                entry.getValue().close();
            } catch (IOException e) {
                Log.e(TAG, "Error closing file descriptor " + entry.getKey(), e);
            }
        }
        openFileDescriptors.clear();
    }

    /**
     * Helper method to find a file by relative path from mmbasic directory
     * Supports paths like "subfolder/file.txt" or just "file.txt"
     */
    private static DocumentFile findFileByPath(String filePath) {
        if (filePath == null || filePath.isEmpty()) {
            return null;
        }

        // Remove any leading "/".
        if (filePath.startsWith("/")) {
            filePath = filePath.substring(1);
        }

        DocumentFile currentDir = instance.mmbasicDirectory;

        // Split the path into components
        String[] pathParts = filePath.split("/");

        // Navigate through directories (all parts except the last)
        for (int i = 0; i < pathParts.length - 1; i++) {
            String dirName = pathParts[i];
            if (!dirName.isEmpty()) {
                DocumentFile nextDir = currentDir.findFile(dirName);
                if (nextDir == null || !nextDir.exists() || !nextDir.isDirectory()) {
                    Log.w(TAG, "findFileByPath: Directory not found: " + dirName +
                        " in path: " + filePath);
                    return null;
                }
                currentDir = nextDir;
                Log.v(TAG, "findFileByPath: Navigated to directory: " + dirName);
            }
        }

        // Find the final file/directory
        String targetName = pathParts[pathParts.length - 1];
        if (targetName.isEmpty()) {
            // Path ended with '/' - return the directory itself
            return currentDir;
        }

        DocumentFile targetFile = currentDir.findFile(targetName);
        Log.v(TAG, "findFileByPath: Looking for '" + targetName + "' in final directory");

        return targetFile;
    }

    /**
     * Alternative method that returns stats as separate values via JNI object
     * This version might be easier to use from C++ if you prefer individual calls
     */
    public static boolean getFileInfo(String filename, long[] outStats) {
        Log.v(TAG, "getFileInfo: " + filename);
        if (!hasDirectoryAccess() || outStats == null || outStats.length < 6) {
            return false;
        }

        // if (filename.equals("/")) {
        //     outStats[0] = 1;     // exists
        //     outStats[1] = 0;     // Not a file
        //     outStats[2] = 1;     // Is a directory
        //     outStats[3] = -1;
        //     outStats[4] = 0;     // TODO: 00:00:00 UTC, January 1, 1970
        //     outStats[5] = 0x11;  // Read and Write permissions

        //     return true;
        // }

        try {
            DocumentFile file = findFileByPath(filename);
            if (file == null || !file.exists()) {
                return false;
            }

            outStats[0] = 1;  // exists
            outStats[1] = file.isFile() ? 1 : 0;
            outStats[2] = file.isDirectory() ? 1 : 0;
            outStats[3] = file.isFile() ? file.length() : -1;
            outStats[4] = file.lastModified();
            outStats[5] = (file.canRead() ? 1 : 0) | (file.canWrite() ? 2 : 0);

            return true;

        } catch (Exception e) {
            Log.e(TAG, "Error getting file info: " + filename, e);
            return false;
        }
    }

    public static long getFreeSpace() {
        Log.v(TAG, "getFreeSpace");

        // TODO: DataDirectory may not be same drive as Documents/mmbasic directory.
        File path = Environment.getDataDirectory();
        StatFs statFs = new StatFs(path.getPath());
        long blockSize = statFs.getBlockSizeLong();
        long availableBlocks = statFs.getAvailableBlocksLong();
        return availableBlocks * blockSize;
    }

    /**
     * Called from native code to rename a file or directory
     * Note: This only supports renaming within the same directory due to SAF limitations
     */
    public static boolean renameFile(String oldName, String newName) {
        final String LOG_PREFIX = "MainActivity#renameFile: ";
        Log.v(TAG, LOG_PREFIX + "oldName=" + oldName + ", newName=" + newName);
        if (!hasDirectoryAccess()) {
            Log.w(TAG, LOG_PREFIX + "No directory access");
            return false;
        }

        if (oldName == null || newName == null || oldName.isEmpty() || newName.isEmpty()) {
            Log.e(TAG, LOG_PREFIX + "Invalid file names");
            return false;
        }

        // Prevent renaming to the same name
        if (oldName.equals(newName)) {
            Log.d(TAG, LOG_PREFIX + "Source and destination are the same");
            return true;
        }

        try {
            // Find the source file/directory
            DocumentFile sourceFile = instance.mmbasicDirectory.findFile(oldName);
            if (sourceFile == null || !sourceFile.exists()) {
                Log.w(TAG, LOG_PREFIX + "Source file not found: " + oldName);
                return false;
            }

            // Check if destination already exists
            DocumentFile destFile = instance.mmbasicDirectory.findFile(newName);
            if (destFile != null && destFile.exists()) {
                Log.w(TAG, LOG_PREFIX + "Destination already exists: " + newName);
                return false;
            }

            boolean result = sourceFile.renameTo(newName);

            if (result) {
                Log.d(TAG, LOG_PREFIX + "Successfully renamed " + oldName + " to " + newName);
            } else {
                Log.e(TAG, LOG_PREFIX + "Failed to rename " + oldName + " to " + newName);
            }

            return result;

        } catch (Exception e) {
            Log.e(TAG, LOG_PREFIX + "Error renaming file: " + oldName + " -> " + newName, e);
            return false;
        }
    }
}
