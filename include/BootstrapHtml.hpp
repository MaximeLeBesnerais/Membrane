//
// Created by maxime on 2/27/25.
//

#ifndef BOOTSTRAPHTML_HPP
    #define BOOTSTRAPHTML_HPP
    #include "Membrane.hpp"
    inline std::string Membrane::getBootstrapHtml(const std::string &initialUrl) {
        return R"html(
        <!DOCTYPE html>
        <html>
        <head>
            <meta charset="UTF-8">
            <title>Membrane App</title>
            <script>
            // Membrane Framework
            const Membrane = {
                currentPage: null,
                pages: {},
                initialLoad: true,
                originalSetInterval: window.setInterval,
                originalSetTimeout: window.setTimeout,
                originalClearInterval: window.clearInterval,
                originalClearTimeout: window.clearTimeout,
                intervals: {},
                timeouts: {},

                // Register a page module
                registerPage(url, initFn) {
                    this.pages[url] = {
                        init: initFn,
                        cleanup: null,
                        active: false
                    };
                },

                // Initialize tracking for the current page
                initPageTracking(url) {
                    // Create collections for this specific page if they don't exist
                    if (!this.intervals[url]) this.intervals[url] = new Set();
                    if (!this.timeouts[url]) this.timeouts[url] = new Set();

                    // Override interval and timeout functions to track by page URL
                    window.setInterval = (...args) => {
                        const id = this.originalSetInterval.apply(window, args);
                        this.intervals[url].add(id);
                        return id;
                    };

                    window.setTimeout = (...args) => {
                        const id = this.originalSetTimeout.apply(window, args);
                        this.timeouts[url].add(id);
                        return id;
                    };

                    window.clearInterval = (id) => {
                        this.originalClearInterval.call(window, id);
                        if (this.intervals[url]) this.intervals[url].delete(id);
                    };

                    window.clearTimeout = (id) => {
                        this.originalClearTimeout.call(window, id);
                        if (this.timeouts[url]) this.timeouts[url].delete(id);
                    };
                },

                // Clean up all intervals and timeouts for a specific page
                cleanupPageTimers(url) {
                    if (this.intervals[url]) {
                        this.intervals[url].forEach(id => {
                            this.originalClearInterval.call(window, id);
                        });
                        this.intervals[url].clear();
                    }

                    if (this.timeouts[url]) {
                        this.timeouts[url].forEach(id => {
                            this.originalClearTimeout.call(window, id);
                        });
                        this.timeouts[url].clear();
                    }

                    // Restore original timer functions
                    window.setInterval = this.originalSetInterval;
                    window.setTimeout = this.originalSetTimeout;
                    window.clearInterval = this.originalClearInterval;
                    window.clearTimeout = this.originalClearTimeout;
                },

                // Navigate to a specific page
                navigate(url) {
                    console.log("Navigating to:", url);

                    // Clean up current page if exists
                    if (this.currentPage) {
                        // Execute custom cleanup if available
                        if (this.pages[this.currentPage] && this.pages[this.currentPage].cleanup) {
                            console.log("Executing custom cleanup for page:", this.currentPage);
                            this.pages[this.currentPage].cleanup();
                            this.pages[this.currentPage].active = false;
                        }

                        // Always clean up page timers
                        console.log("Cleaning up timers for page:", this.currentPage);
                        this.cleanupPageTimers(this.currentPage);
                    }

                    // Fetch new page
                    window.fetchPage(url)
                        .then(response => {
                            if (response.status !== "success") {
                                throw new Error("Failed to load page");
                            }

                            // Clear existing content
                            const contentElement = document.getElementById('content');
                            contentElement.innerHTML = '';

                            // Create a new parser and parse the HTML
                            const parser = new DOMParser();
                            const doc = parser.parseFromString(response.content, 'text/html');

                            // Append all body children to our content div
                            Array.from(doc.body.children).forEach(child => {
                                contentElement.appendChild(document.importNode(child, true));
                            });

                            // Execute all scripts manually
                            const scripts = contentElement.querySelectorAll('script');
                            scripts.forEach(oldScript => {
                                const newScript = document.createElement('script');
                                Array.from(oldScript.attributes).forEach(attr => {
                                    newScript.setAttribute(attr.name, attr.value);
                                });
                                newScript.textContent = oldScript.textContent;
                                oldScript.parentNode.replaceChild(newScript, oldScript);
                            });

                            // Set up timer tracking for the new page
                            this.initPageTracking(url);

                            // Initialize the page if registered
                            if (this.pages[url]) {
                                console.log("Initializing page:", url);
                                const cleanupFn = this.pages[url].init();
                                this.pages[url].cleanup = cleanupFn || (() => {});
                                this.pages[url].active = true;
                            } else {
                                console.log("No registered handler for page:", url);
                                // Set up automatic cleanup for any intervals/timeouts created by the page
                                this.setupAutoCleanup(url);
                            }

                            this.currentPage = url;
                        })
                        .catch(error => {
                            console.error("Navigation error:", error);
                            document.getElementById('content').innerHTML =
                                `<h1>Error Loading Page</h1><p>${error.message}</p>`;
                        });
                },

                // Set up automatic cleanup for pages without explicit registration
                setupAutoCleanup(url) {
                    // Register cleanup function that will be called on navigation
                    this.pages[url] = {
                        init: () => {},
                        cleanup: () => {
                            // Any additional cleanup logic can go here
                            console.log("Running auto-cleanup for page:", url);
                        },
                        active: true
                    };
                }
            };

            // Global navigation function
            function changePage(url) {
                Membrane.navigate(url);
            }

            // Initialize on DOM load
            document.addEventListener('DOMContentLoaded', function() {
                console.log("DOM loaded, initializing Membrane");
                changePage(')html" + initialUrl + R"html(');
            });
            </script>
        </head>
        <body>
            <div id="content"></div>
        </body>
        </html>
        )html";
    }
#endif //BOOTSTRAPHTML_HPP
