#include <fstream>
#include <iostream>
#include <sstream>
#include "json/json.h"

class JsonHandler
{
private:
    Json::Value root;
public:
    bool readJsonFile(const std::string& filename);
    bool readJsonString(const std::string& data);
    bool writeJsonFile(const std::string& filename);
    void clear();
    std::string writeJsonString();
    Json::Value& operator[](const std::string& key);
};