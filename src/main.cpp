#include "Membrane_lib/Membrane.hpp"
#include "resource_init.hpp"

int main() {
    Membrane app("SSH Key Gen", "index.html", 620, 880, WEBVIEW_HINT_FIXED);
    initialize_resources(app);

    app.run();
    return 0;
}