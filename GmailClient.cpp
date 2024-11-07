#include "GmailClient.h"

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

std::string GmailClient::base64UrlDecode(std::string const& in) {
    const char base64_url_alphabet[] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '-', '_'
    };
    std::string out;
    std::vector<int> T(256, -1);
    unsigned int i;
    for (i =0; i < 64; i++) 
        T[base64_url_alphabet[i]] = i;
    int val = 0, valb = -8;
    for (i = 0; i < in.length(); i++) {
        unsigned char c = in[i];
        if (T[c] == -1) 
            break;
        val = (val<<6) + T[c];
        valb += 6;
        if (valb >= 0) {
        out.push_back(char((val>>valb)&0xFF));
        valb -= 8;
        }
    }
    return out;
}
