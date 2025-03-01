#include <nlohmann/json.hpp>
#include "Membrane_lib/Membrane.hpp"
#include "resource_init.hpp"

using json = nlohmann::json;


int main() {
    Membrane app("Membrane App", "index.html", 800, 600, WEBVIEW_HINT_NONE);
    initialize_resources(app);
    app.run();
    return 0;
}
