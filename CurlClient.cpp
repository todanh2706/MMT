#include "CurlClient.h"

CurlClient::CurlClient()
{
    this->curl = curl_easy_init();
}

CurlClient::~CurlClient()
{
    if (this->curl)
    {
        curl_easy_cleanup(this->curl);
        this->curl = nullptr;
    }
}

size_t CurlClient::WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output)
{
    size_t totalSize = size * nmemb;
    output->append((char*)contents, totalSize);
    return totalSize;
}

std::string CurlClient::postRequest(const std::string& url, const std::string& postData)
{
    if (!curl)
        return "";
    std::string readBuffer;
    curl_easy_setopt(this->curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(this->curl, CURLOPT_POSTFIELDS, postData.c_str());
    curl_easy_setopt(this->curl, CURLOPT_WRITEFUNCTION, &CurlClient::WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    CURLcode res = curl_easy_perform(this->curl);
    if(res != CURLE_OK)
    {
        std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        return "";
    }
    return readBuffer;
}

std::string CurlClient::getRequest(const std::string& url, const std::string& authHeader)
{
    if (!this->curl)
        return "";
    curl_slist* header = nullptr;
    CURLcode response;
    header = curl_slist_append(header, authHeader.c_str());
    std::string readBuffer;

    curl_easy_setopt(this->curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(this->curl, CURLOPT_HTTPHEADER, header);
    curl_easy_setopt(this->curl, CURLOPT_WRITEFUNCTION, &CurlClient::WriteCallback);
    curl_easy_setopt(this->curl, CURLOPT_WRITEDATA, &readBuffer);

    response = curl_easy_perform(this->curl);

    if (response != CURLE_OK) 
        std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(response) << std::endl;
    return readBuffer;
}