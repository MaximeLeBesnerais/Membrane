#include <nlohmann/json.hpp>
#include "Membrane_lib/Membrane.hpp"
#include "aggregate.hpp"

using json = nlohmann::json;


int main() {
    Membrane app("Membrane App", "index.html", 800, 600, WEBVIEW_HINT_NONE);
    app.add_vfs("index.html", res_index_html, res_index_html_len);
    app.add_vfs("about.html", res_about_html, res_about_html_len);
    app.add_vfs("style.css", res_style_css, res_style_css_len);
    app.run();
    return 0;
}
