#include <webview/webview.h>
#include <nlohmann/json.hpp>
#include "vfs/vfs.hpp"
#include "aggregate.hpp"

static const std::string index_h(reinterpret_cast<const char*>(res_index_html), res_index_html_len);

using json = nlohmann::json;

int main() {
    webview::webview window(true, nullptr);
    window.set_title("Membrane");
    window.set_size(800, 600, WEBVIEW_HINT_NONE);
    VirtualFileSystem vfs;

    vfs.add_file("/index.html", res_index_html, res_index_html_len);
    vfs.add_file("/about.html", res_about_html, res_about_html_len);

    window.set_html(index_h);
    window.run();
    return 0;
}
