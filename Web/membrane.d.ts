/**
 * Result object returned by Membrane API operations
 */
interface MembraneResult {
    status: "success" | "error";
    message: string;
    data: any | null;
}

/**
 * VFS (Virtual File System) API interface
 */
interface VfsAPI {
    /**
     * Creates a new custom VFS
     * @param name Name of the VFS to create
     * @param persistenceDir Optional directory for persisting files to disk
     */
    createCustomVfs(name: string, persistenceDir?: string | null): Promise<MembraneResult>;

    /**
     * Adds a file to a VFS
     * @param vfsName Name of the VFS to add the file to (empty string for default VFS)
     * @param path Path within the VFS to store the file
     * @param content Content of the file
     */
    addFile(vfsName: string, path: string, content: string): Promise<MembraneResult>;

    /**
     * Saves a specific VFS to disk (only works for persistent VFS)
     * @param vfsName Name of the VFS to save
     */
    saveVfsToDisk(vfsName: string): Promise<MembraneResult>;

    /**
     * Saves all persistent VFS instances to disk
     */
    saveAllVfsToDisk(): Promise<MembraneResult>;
}

/**
 * Main Membrane API interface
 */
interface MembraneAPI {
    /**
     * Virtual File System operations
     */
    vfs: VfsAPI;

    /**
     * Opens a URL in the default system browser
     * @param url URL to open
     */
    openExternalUrl(url: string): Promise<MembraneResult>;

    /**
     * Saves a file to the local filesystem
     * @param path Path where to save the file
     * @param content Content of the file
     * @param mime MIME type of the file
     */
    saveFile(path: string, content: string, mime: string): Promise<MembraneResult>;
}

declare global {
    interface Window {
        membrane: MembraneAPI;
    }
}

// Create a variable for the export
declare const membrane: MembraneAPI;

// Export the type
export default membrane;