// Membrane.js - React Integration Library with Custom Import Support
// This provides a clean interface between your React app and the C++ backend,
// and adds custom import functions to load modules via the membrane:// protocol.

class Membrane {
    constructor() {
        // Store registered pages and API handlers
        this.pages = new Map();
        this.currentPage = null;
        this.handlers = {};

        // Bind methods
        this.registerPage = this.registerPage.bind(this);
        this.navigate = this.navigate.bind(this);
        this.callNative = this.callNative.bind(this);
        this.registerHandler = this.registerHandler.bind(this);
        this.loadMembraneResource = this.loadMembraneResource.bind(this);
        this.dynamicImport = this.dynamicImport.bind(this);

        // Make available globally for C++ to call
        window.Membrane = this;

        // Set up global API receiver for C++ callbacks
        window.receiveFromNative = this.receiveFromNative.bind(this);
    }

    // Register a React component/page with optional init and cleanup
    registerPage(path, component, options = {}) {
        this.pages.set(path, {
            component,
            init: options.init || (() => {}),
            cleanup: options.cleanup || (() => {}),
            active: false
        });
        console.log(`Registered page: ${path}`);
        return this;
    }

    // Navigate to a registered page
    navigate(path) {
        console.log(`Navigating to: ${path}`);
        // Run cleanup on current page if any
        if (this.currentPage && this.pages.has(this.currentPage)) {
            const page = this.pages.get(this.currentPage);
            if (page.active && typeof page.cleanup === 'function') {
                console.log(`Running cleanup for: ${this.currentPage}`);
                page.cleanup();
            }
            page.active = false;
        }
        // Set new current page
        this.currentPage = path;
        // Initialize the new page if registered
        if (this.pages.has(path)) {
            const page = this.pages.get(path);
            if (typeof page.init === 'function') {
                console.log(`Initializing page: ${path}`);
                page.init();
            }
            page.active = true;
        } else {
            console.warn(`No registered handler for page: ${path}`);
        }
        // Tell the C++ backend about the navigation
        this.callNative('onNavigate', { path });
    }

    // Call a C++ function exposed through the webview binding
    async callNative(method, params = {}) {
        // Ensure the method exists in the window context (bound by C++)
        if (typeof window[method] !== 'function') {
            console.error(`Native method not available: ${method}`);
            return null;
        }
        try {
            // Call the C++ bound function
            const resultJson = await window[method](JSON.stringify(params));
            return JSON.parse(resultJson);
        } catch (error) {
            console.error(`Error calling native method ${method}:`, error);
            return null;
        }
    }

    // Register a handler for C++ callbacks
    registerHandler(name, callback) {
        this.handlers[name] = callback;
        return this;
    }

    // Receive data from C++ (called by the global receiveFromNative function)
    receiveFromNative(type, dataJson) {
        try {
            const data = JSON.parse(dataJson);
            console.log(`Received from native: ${type}`, data);
            // Call registered handler if available
            if (this.handlers[type]) {
                this.handlers[type](data);
            }
        } catch (error) {
            console.error(`Error processing native callback (${type}):`, error);
        }
    }

    // Fetch resources from the C++ side using the fetchPage binding
    async fetchResource(url) {
        return this.callNative('fetchPage', url);
    }

    // Get current timestamp from the native side
    async getCurrentTime() {
        return this.callNative('getCurrentTime');
    }

    // Custom Import: Load a membrane:// resource as text
    async loadMembraneResource(url) {
        if (url.startsWith("membrane://")) {
            const responseJson = await this.fetchResource(url);
            const response = JSON.parse(responseJson);
            if (response.status === "success") {
                return response.content;
            } else {
                throw new Error("Failed to load resource: " + response.message);
            }
        } else {
            // Fallback to normal fetch if not using the membrane protocol
            const resp = await fetch(url);
            return await resp.text();
        }
    }

    // Dynamic import for membrane:// URLs.
    // It converts the fetched module content into a Blob URL, then imports it as an ES module.
    async dynamicImport(url) {
        const scriptContent = await this.loadMembraneResource(url);
        const blob = new Blob([scriptContent], { type: 'application/javascript' });
        const blobUrl = URL.createObjectURL(blob);
        return import(blobUrl);
    }
}

// Export the singleton instance
export default new Membrane();
