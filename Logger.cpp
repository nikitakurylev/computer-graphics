#include <iostream>
#include "Logger.hpp"

void ConsoleLogger::LogInfo(const std::string& message)
{
    std::cout << "[INFO] " << message << std::endl;
}

void ConsoleLogger::LogError(const std::string& message)
{
    std::cerr << "[ERROR] " << message << std::endl;
}

void ConsoleLogger::LogWarning(const std::string& message)
{
    std::cout << "[WARNING] " << message << std::endl;
}


void EmptyLogger::LogInfo(const std::string& message)
{
}

void EmptyLogger::LogError(const std::string& message)
{
}

void EmptyLogger::LogWarning(const std::string& message)
{
}