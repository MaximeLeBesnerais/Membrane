// membrane.js
(function() {
    /**
     * Membrane.js - JavaScript API for interacting with the Membrane C++ core
     */
    const membrane = {
        /**
         * Virtual File System (VFS) operations
         */
        vfs: {
            /**
             * Creates a new custom VFS
             * @param {string} name - Name of the VFS to create
             * @param {string|null} persistenceDir - Optional directory for persisting files to disk
             * @returns {Promise<Object>} Result of the operation
             */
            createCustomVfs: async (name, persistenceDir = null) => {
                if (persistenceDir) {
                    return window.createCustomVfs(name, persistenceDir);
                } else {
                    return window.createCustomVfs(name);
                }
            },

            /**
             * Adds a file to a VFS
             * @param {string} vfsName - Name of the VFS to add the file to (empty string for default VFS)
             * @param {string} path - Path within the VFS to store the file
             * @param {string} content - Content of the file
             * @returns {Promise<Object>} Result of the operation
             */
            addFile: async (vfsName, path, content) => {
                return window.addFileToVfs(vfsName, path, content);
            },

            /**
             * Saves a specific VFS to disk (only works for persistent VFS)
             * @param {string} vfsName - Name of the VFS to save
             * @returns {Promise<Object>} Result of the operation
             */
            saveVfsToDisk: async (vfsName) => {
                return window.saveVfsToDisk(vfsName);
            },

            /**
             * Saves all persistent VFS instances to disk
             * @returns {Promise<Object>} Result of the operation
             */
            saveAllVfsToDisk: async () => {
                return window.saveAllVfsToDisk();
            }
        },

        /**
         * Opens a URL in the default system browser
         * @param {string} url - URL to open
         * @returns {Promise<Object>} Result of the operation
         */
        openExternalUrl: async (url) => {
            return window.openExternalUrl(url);
        },

        /**
         * Saves a file to the local filesystem
         * @param {string} path - Path where to save the file
         * @param {string} content - Content of the file
         * @param {string} mime - MIME type of the file
         * @returns {Promise<Object>} Result of the operation
         */
        saveFile: async (path, content, mime) => {
            return window.saveFile(path, content, mime);
        }
    };

    // Expose membrane globally
    window.membrane = membrane;
})();
