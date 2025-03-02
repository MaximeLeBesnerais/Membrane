// membrane-api.ts
/**
 * Result object returned by Membrane API operations
 */
export interface MembraneResult {
    status: "success" | "error";
    message: string;
    data: any | null;
}

/**
 * VFS (Virtual File System) API interface
 */
export interface VfsAPI {
    createCustomVfs(name: string, persistenceDir?: string | null): Promise<MembraneResult>;
    addFile(vfsName: string, path: string, content: string): Promise<MembraneResult>;
    saveVfsToDisk(vfsName: string): Promise<MembraneResult>;
    saveAllVfsToDisk(): Promise<MembraneResult>;
}

/**
 * Main Membrane API interface
 */
export interface MembraneAPI {
    vfs: VfsAPI;
    openExternalUrl(url: string): Promise<MembraneResult>;
    saveFile(path: string, content: string, mime: string): Promise<MembraneResult>;
}

// Declare membrane as available on the global window
declare global {
    interface Window {
        membrane: MembraneAPI;
    }
}

// Simple wrapper to access the global membrane object
const membrane = window.membrane;
export default membrane;
