#include "JsonHandler.h"

void JsonHandler::clear()
{
    this->root.clear();
}

bool JsonHandler::readJsonFile(const std::string& filename)
{
    std::ifstream fin(filename, std::ifstream::binary);
    if (!fin.is_open())
    {
        std::cerr << "Can't open file: " << filename << std::endl;
        return false;
    }
    Json::CharReaderBuilder reader;
    std::string errs;
    if (!Json::parseFromStream(reader, fin, &this->root, &errs))
    {
        std::cerr << "Cannot parse the file" << std::endl << errs;
        return false;
    }
    fin.close();
    return true;
}

bool JsonHandler::readJsonString(const std::string& data)
{
    std::stringstream ss(data);
    Json::CharReaderBuilder reader;
    std::string errs;
    if (!Json::parseFromStream(reader, ss, &this->root, &errs))
    {
        std::cerr << "Cannot parse the given string" << std::endl << errs;
        return false;
    }
    return true;
}

bool JsonHandler::writeJsonFile(const std::string& filename)
{
    std::ofstream fout(filename, std::ofstream::trunc);
    if (!fout.is_open())
    {
        std::cerr << "Can't open file: " << filename << std::endl;
        return false;
    }
    std::string jsonString = this->writeJsonString();
    fout << jsonString;
    fout.close();
    return true;
}

std::string JsonHandler::writeJsonString()
{
    Json::StreamWriterBuilder writer;
    return Json::writeString(writer, this->root);
}

Json::Value& JsonHandler::operator[](const std::string& key)
{
    return this->root[key];
}