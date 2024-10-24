#pragma once
#include <iostream>
#include <curl/curl.h>

class CurlClient
{
protected:
    CURL *curl;
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output);
public:
    CurlClient();
    ~CurlClient();
    std::string postRequest(const std::string& url, const std::string& postData);
    std::string getRequest(const std::string& url, const std::string& header);
};