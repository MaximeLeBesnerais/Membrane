//
// Created by maxime on 2/27/25.
//

#include "Membrane.hpp"
#include "BootstrapHtml.hpp"

Membrane::Membrane(const bool debug, const std::string &title, const int width, const int height):
m_webview(new webview::webview(debug, nullptr))
{
    m_webview->set_title(title);
    m_webview->set_size(width, height, WEBVIEW_HINT_NONE);

    // Register the core fetchPage method
    m_webview->bind("fetchPage", [this](const std::string& req) -> std::string {
        const std::string url = parseUrl(req);
        std::cout << "Fetching page: " << url << std::endl;

        if (m_resources.contains(url)) {
            json result;
            result["status"] = "success";
            result["content"] = m_resources[url];
            return result.dump();
        }

        // Check if we have a resource handler for this URL scheme
        std::string scheme = getScheme(url);
        if (m_resourceHandlers.contains(scheme)) {
            try {
                std::string content = m_resourceHandlers[scheme](url);
                json result;
                result["status"] = "success";
                result["content"] = content;
                return result.dump();
            } catch (const std::exception& e) {
                json result;
                result["status"] = "error";
                result["content"] = std::string("<h1>Error</h1><p>") + e.what() + "</p>";
                return result.dump();
            }
        }

        json result;
        result["status"] = "error";
        result["content"] = "<h1>404 Not Found</h1><p>The requested resource was not found.</p>";
        return result.dump();
    });
}

void Membrane::addResource(const std::string &url, const std::string &content) {
    m_resources[url] = content;
}

void Membrane::registerResourceHandler(const std::string &scheme, const ResourceCallback &callback) {
    m_resourceHandlers[scheme] = callback;
}

void Membrane::registerAPI(const std::string &name, APICallback callback) const {
    m_webview->bind(name, callback);
}

void Membrane::setTitle(const std::string &title) const {
    m_webview->set_title(title);
}

void Membrane::setSize(int width, int height, int hints) const {
    m_webview->set_size(width, height, static_cast<webview_hint_t>(hints));
}

void Membrane::run(const std::string &bootstrapUrl) {
    if (!bootstrapUrl.empty()) {
        const std::string html = getBootstrapHtml(bootstrapUrl);
        m_webview->set_html(html);
    } else {
        m_webview->set_html(getDefaultBootstrapHtml());
    }
    m_webview->run();
}

std::string Membrane::parseUrl(const std::string &req) {
    std::string url = req;
    if (url.size() >= 2 && url[0] == '[' && url[url.size()-1] == ']') {
        url = url.substr(1, url.size() - 2);
    }
    if (url.size() >= 2 && url[0] == '"' && url[url.size()-1] == '"') {
        url = url.substr(1, url.size() - 2);
    }
    return url;
}

std::string Membrane::getScheme(const std::string &url) {
    size_t pos = url.find("://");
    if (pos != std::string::npos) {
        return url.substr(0, pos);
    }
    return "";
}

std::string Membrane::getDefaultBootstrapHtml() {
    return getBootstrapHtml("membrane://index.html");
}
