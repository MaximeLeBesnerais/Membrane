
#include "Membrane_lib/Membrane.hpp"
#include "resource_init.hpp"

int main() {
    // Create a Membrane app with the title "My Membrane App", the entry point
    // "index.html", and the window size 620x920. The Hint can be None, Fixed,
    // Max or Min (ca, resize freely, cannot resize, given size is the maximum
    // size, given size is the minimum size)
    Membrane app("My Membrane App", "index.html", 620, 920, WEBVIEW_HINT_NONE);
    // Initialize the resources from your react app xxd output (this process is
    // explained in the README.md)
    initialize_resources(app);
    // Check if the resources are zipped and unzip them if they are zipped
    app.checkAndUnzip();

    // This line is essential to run the app. No window will be displayed
    // without this line.
    app.run();
    app.Membrase();
    return 0;
}
