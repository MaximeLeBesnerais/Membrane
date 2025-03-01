#include <webview/webview.h>
#include <nlohmann/json.hpp>
#include "about.hpp"
#include "index.hpp"

static const std::string about(reinterpret_cast<const char*>(res_about_html), res_about_html_len);
static const std::string index_h(reinterpret_cast<const char*>(res_index_html), res_index_html_len);
static std::unordered_map<std::string, std::string> pages = {
    {"membrane://about.html", about},
    {"membrane://index.html", index_h}
};

std::string error_page(const std::string &error) {
    std::string error1 = R"html(
        <!DOCTYPE html>
        <html>
        <head>
            <title>Error</title>
            <viewport content="width=device-width, initial-scale=1">
            <style>
                body {
                    font-family: Arial, sans-serif;
                    margin: 0;
                    padding: 0;
                    background-color: #f0f0f0;
                    color: #333;
                }
                .container {
                    display: flex;
                    justify-content: center;
                    align-items: center;
                    height: 100vh;
                }
                .content {
                    text-align: center;
                }
                h1 {
                    font-size: 2em;
                    margin: 0;
                    padding: 0;
                }
                p {
                    font-size: 1.5em;
                    margin: 0;
                    padding: 0;
                }
            </style>
        </head>
        <body>
            <div class="container">
                <div class="content">
                    <h1>Error</h1>
                    <p>)html";
    error1 += error;
    error1 += R"html(</p>
                </div>
            </div>
        </body>
        </html>
    )html";
    return error1;
}

using json = nlohmann::json;

int main() {
    webview::webview window(true, nullptr);
    window.set_title("Membrane");
    window.set_size(800, 600, WEBVIEW_HINT_NONE);

    window.bind("changePage", [](const std::string &page) {
        if (pages.contains(page)) {
            json response;
            response["status"] = "success";
            response["content"] = pages[page];
            return response.dump();
        }
        json response;
        response["status"] = "error";
        response["content"] = error_page("Page not found");
        return response.dump();
    });
    window.set_html(index_h);
    window.run();
    return 0;
}
