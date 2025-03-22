
#include "Membrane_lib/Membrane.hpp"
#include "resource_init.hpp"
#ifndef BIN_NAME_MEMBRANE
#define BIN_NAME_MEMBRANE "My Membrane App"
#endif

int main() {
    // Create a Membrane app with the title "My Membrane App", the entry point
    // "index.html", and the window size 620x920. The Hint can be None, Fixed,
    // Max or Min (ca, resize freely, cannot resize, given size is the maximum
    // size, given size is the minimum size)
    Membrane app(BIN_NAME_MEMBRANE, "index.html", 1420, 980, WEBVIEW_HINT_NONE);
    // Initialize the resources from your react app xxd output (this process is
    // explained in the README.md)
    initialize_resources(app);
    // Check if the resources are zipped and unzip them if they are zipped
    app.checkAndUnzip();

    // This line is essential to run the app. No window will be displayed
    // without this line.
    app.run();
    app.Terminate();
    return 0;
}
