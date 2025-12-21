#pragma once

#include <string>

class HTTPClient {
private:
    static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp);

public:
    static std::string post(const std::string& url, const std::string& data);
};
