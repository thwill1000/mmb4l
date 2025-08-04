package com.sockpuppetstudios.mmb4a;

import org.libsdl.app.SDLActivity;
import android.content.Intent;
import android.content.UriPermission;
import android.net.Uri;
import android.provider.DocumentsContract;
import android.content.ContentResolver;
import android.database.Cursor;
import android.provider.OpenableColumns;
import java.io.InputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

public class MainActivity extends SDLActivity {
    private static final int REQUEST_CODE_OPEN_DOCUMENT_TREE = 1001;
    private static final int REQUEST_CODE_OPEN_DOCUMENT = 1002;
    
    private static MainActivity instance;
    private Uri documentsTreeUri = null;

    private static final String LOG_TAG = "MMB4A";
    
    @Override
    protected void onCreate(android.os.Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        instance = this;
        
        // Debug existing permissions on startup
        debugExistingPermissions();
        
        // Try to restore previously granted permissions
        restoreExistingPermissions();
    }
    
    @Override
    protected String[] getLibraries() {
        return new String[] {
            "SDL2",
            "main"
        };
    }
    
    // JNI methods called from native code
    public static void requestDocumentsAccess() {
        if (instance != null) {
            instance.requestDocumentsAccessInternal();
        }
    }
    
    public static String[] listDocumentsFiles() {
        if (instance != null && instance.documentsTreeUri != null) {
            return instance.listFilesInDirectory(instance.documentsTreeUri);
        }
        return new String[0];
    }
    
    public static byte[] readDocumentFile(String fileName) {
        if (instance != null && instance.documentsTreeUri != null) {
            return instance.readFileFromDocuments(fileName);
        }
        return null;
    }
    
    public static boolean hasDocumentsAccess() {
        return instance != null && instance.documentsTreeUri != null;
    }
    
    private void requestDocumentsAccessInternal() {
        Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT_TREE);
        
        // Try to start in the Documents folder
        try {
            Uri documentsUri = DocumentsContract.buildDocumentUri(
                "com.android.externalstorage.documents", 
                "primary:Documents"
            );
            intent.putExtra(DocumentsContract.EXTRA_INITIAL_URI, documentsUri);
        } catch (Exception e) {
            // Fallback without initial URI if it fails
        }
        
        intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION | 
                       Intent.FLAG_GRANT_WRITE_URI_PERMISSION |
                       Intent.FLAG_GRANT_PERSISTABLE_URI_PERMISSION);
        
        startActivityForResult(intent, REQUEST_CODE_OPEN_DOCUMENT_TREE);
    }
    
    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        
        if (requestCode == REQUEST_CODE_OPEN_DOCUMENT_TREE && resultCode == RESULT_OK) {
            if (data != null && data.getData() != null) {
                documentsTreeUri = data.getData();
                
                // Take persistent permission
                getContentResolver().takePersistableUriPermission(
                    documentsTreeUri,
                    Intent.FLAG_GRANT_READ_URI_PERMISSION | Intent.FLAG_GRANT_WRITE_URI_PERMISSION
                );
                
                // Notify native code that access was granted
                nativeOnDocumentsAccessGranted();
            } else {
                // Notify native code that access was denied
                nativeOnDocumentsAccessDenied();
            }
        }
    }
    
    private String[] listFilesInDirectory(Uri treeUri) {
        List<String> fileNames = new ArrayList<>();
        ContentResolver resolver = getContentResolver();
        
        try {
            Uri childrenUri = DocumentsContract.buildChildDocumentsUriUsingTree(
                treeUri, DocumentsContract.getTreeDocumentId(treeUri)
            );
            
            try (Cursor cursor = resolver.query(
                childrenUri,
                new String[]{DocumentsContract.Document.COLUMN_DISPLAY_NAME, 
                           DocumentsContract.Document.COLUMN_MIME_TYPE},
                null, null, null
            )) {
                if (cursor != null) {
                    while (cursor.moveToNext()) {
                        String displayName = cursor.getString(0);
                        String mimeType = cursor.getString(1);
                        
                        // Only include files, not directories
                        if (!DocumentsContract.Document.MIME_TYPE_DIR.equals(mimeType)) {
                            fileNames.add(displayName);
                        }
                    }
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
        
        return fileNames.toArray(new String[0]);
    }
    
    private byte[] readFileFromDocuments(String fileName) {
        if (documentsTreeUri == null) return null;
        
        ContentResolver resolver = getContentResolver();
        
        try {
            // Find the file
            Uri childrenUri = DocumentsContract.buildChildDocumentsUriUsingTree(
                documentsTreeUri, DocumentsContract.getTreeDocumentId(documentsTreeUri)
            );
            
            try (Cursor cursor = resolver.query(
                childrenUri,
                new String[]{DocumentsContract.Document.COLUMN_DISPLAY_NAME,
                           DocumentsContract.Document.COLUMN_DOCUMENT_ID},
                null, null, null
            )) {
                if (cursor != null) {
                    while (cursor.moveToNext()) {
                        String displayName = cursor.getString(0);
                        if (fileName.equals(displayName)) {
                            String documentId = cursor.getString(1);
                            Uri fileUri = DocumentsContract.buildDocumentUriUsingTree(
                                documentsTreeUri, documentId
                            );
                            
                            // Read the file
                            try (InputStream inputStream = resolver.openInputStream(fileUri)) {
                                if (inputStream != null) {
                                    return inputStream.readAllBytes();
                                }
                            }
                            break;
                        }
                    }
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
        
        return null;
    }
    
    // Add this method to MainActivity for debugging
    public static void debugExistingPermissions() {
        if (instance != null) {
            List<UriPermission> permissions = instance.getContentResolver().getPersistedUriPermissions();
            android.util.Log.i(LOG_TAG, "Found " + permissions.size() + " persisted permissions");
            for (UriPermission permission : permissions) {
                android.util.Log.i(LOG_TAG, "Permission URI: " + permission.getUri().toString());
            }
        }
    }
    
    // Try to restore previously granted Documents access
    private void restoreExistingPermissions() {
        List<UriPermission> permissions = getContentResolver().getPersistedUriPermissions();
        
        for (UriPermission permission : permissions) {
            Uri uri = permission.getUri();
            String uriString = uri.toString();
            
            // Look for Documents folder permissions
            if (uriString.contains("Documents") || uriString.contains("primary")) {
                documentsTreeUri = uri;
                android.util.Log.i(LOG_TAG, "Restored Documents access: " + uriString);
                nativeOnDocumentsAccessGranted();
                break;
            }
        }
        
        if (documentsTreeUri == null) {
            android.util.Log.i(LOG_TAG, "No existing Documents permissions found");
        }
    }
    
    // Method to ensure permissions are saved before exit
    public static void ensurePermissionsPersisted() {
        if (instance != null && instance.documentsTreeUri != null) {
            try {
                // Force persistence of URI permissions
                instance.getContentResolver().takePersistableUriPermission(
                    instance.documentsTreeUri,
                    Intent.FLAG_GRANT_READ_URI_PERMISSION | Intent.FLAG_GRANT_WRITE_URI_PERMISSION
                );
                
                // Give Android time to save the permissions
                try {
                    Thread.sleep(100);
                } catch (InterruptedException e) {
                    // Ignore
                }
            } catch (Exception e) {
                // Permission might already be taken or invalid
            }
        }
    }
    
    @Override
    protected void onDestroy() {
        ensurePermissionsPersisted();
        super.onDestroy();
    }
    
    @Override
    protected void onPause() {
        ensurePermissionsPersisted();
        super.onPause();
    }
    
    // Native methods to implement in your C/C++ code
    public native void nativeOnDocumentsAccessGranted();
    public native void nativeOnDocumentsAccessDenied();
}
