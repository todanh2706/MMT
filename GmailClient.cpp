#include "GmailClient.h"
#include "json/json.h"
#include <fstream>
#include <sstream>

GmailClient::GmailClient(std::string filename)
{
    JsonHandler JsonBuffer;
    JsonBuffer.readJsonFile(filename);
    this->accessToken = JsonBuffer["accessToken"].asString();
    this->clientID = JsonBuffer["clientID"].asString();
    this->clientSecret = JsonBuffer["clientSecret"].asString();
    this->refreshToken = JsonBuffer["refreshToken"].asString();
}

void GmailClient::setClientID(std::string clientID)
{
    this->clientID = clientID;
}

void GmailClient::setClientSecret(std::string clientSecret)
{
    this->clientSecret = clientSecret;
}

void GmailClient::setAccessToken(std::string accessToken)
{
    this->accessToken = accessToken;
}

void GmailClient::setRefreshToken(std::string refreshToken)
{
    this->refreshToken = refreshToken;
}

void GmailClient::renewAccessToken()
{
    std::string url = "https://oauth2.googleapis.com/token";
    std::string postData = "grant_type=refresh_token&refresh_token=" + this->refreshToken + "&client_id=" + this->clientID + "&client_secret=" + this->clientSecret;
    std::string response = this->postRequest(url, postData);
    JsonHandler JsonBuffer;
    JsonBuffer.readJsonString(response);
    this->accessToken = JsonBuffer["access_token"].asString();
}

void GmailClient::writeAccessToken(std::string filename)
{
    JsonHandler JsonBuffer;
    JsonBuffer["accessToken"] = this->accessToken;
    JsonBuffer["refreshToken"] = this->refreshToken;
    JsonBuffer["clientID"] = this->clientID;
    JsonBuffer["clientSecret"] = this->clientSecret;
    JsonBuffer.writeJsonFile(filename);
}

void GmailClient::readEmail(const std::string& id)
{
    std::string url = "https://www.googleapis.com/gmail/v1/users/me/messages/" + id;
    std::string authHeader = "Authorization: Bearer " + this->accessToken;
    std::string response = this->getRequest(url, authHeader);
    JsonHandler JsonBuffer;
    JsonBuffer.readJsonString(response);
    std::string subject, body;
    for (const auto& header: JsonBuffer["payload"]["headers"])
    {
        if (header["name"].asString() == "Subject")
        {
            subject = header["value"].asString();
            break;
        }
    }
    body = this->base64UrlDecode(JsonBuffer["payload"]["parts"][0]["body"]["data"].asString());
    std::cout << "Subject: " << subject << std::endl << "Body: " << body << std::endl << std::endl;
}

void GmailClient::listUnreadEmail()
{
    std::string url = "https://www.googleapis.com/gmail/v1/users/me/messages?q=is:unread";
    std::string authHeader = "Authorization: Bearer " + this->accessToken;
    std::string response = this->getRequest(url, authHeader);
    JsonHandler JsonBuffer;
    JsonBuffer.readJsonString(response);
    for (const auto& message: JsonBuffer["messages"])
    {
        this->readEmail(message["id"].asString());
    }
}
bool GmailClient::isBase64(unsigned char c)
{
    return (isalnum(c) || (c == '+') || (c == '/'));
}

std::string GmailClient::base64UrlDecode(std::string const& encoded_string) {
    std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ""abcdefghijklmnopqrstuvwxyz""0123456789+/";
    int in_len = encoded_string.size();
    int i = 0;
    int j = 0;
    int in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];
    std::string ret;

    while (in_len-- && ( encoded_string[in_] != '=') && this->isBase64(encoded_string[in_])) {
        char_array_4[i++] = encoded_string[in_]; in_++;
        if (i ==4) {
        for (i = 0; i <4; i++)
            char_array_4[i] = base64_chars.find(char_array_4[i]);

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (i = 0; (i < 3); i++)
            ret += char_array_3[i];
        i = 0;
        }
    }

    if (i) {
        for (j = i; j <4; j++)
        char_array_4[j] = 0;

        for (j = 0; j <4; j++)
        char_array_4[j] = base64_chars.find(char_array_4[j]);

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
    }

    return ret;
}
