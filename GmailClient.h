#include <iostream>
#include <curl/curl.h>
#include "CurlClient.h"
#include "JsonHandler.h"

class GmailClient : public CurlClient
{
private:
    std::string clientID, clientSecret;
    std::string accessToken, refreshToken;
    std::string base64UrlDecode(const std::string& input);
    bool isBase64(unsigned char c);
public:
    GmailClient(std::string fileName);
    GmailClient(std::string id, std::string secret, std::string access, std::string refresh) : clientID(id), clientSecret(secret), accessToken(access), refreshToken(refresh) {};
    void setClientID(std::string clientID);
    void setClientSecret(std::string clientSecret);
    void setAccessToken(std::string accessToken);
    void setRefreshToken(std::string refreshToken);
    void renewAccessToken();
    void writeAccessToken(std::string filename);
    void listUnreadEmail();
    void readEmail(const std::string& id);
};