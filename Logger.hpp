#pragma once
#include <string>

class ILogger
{
public:
    virtual ~ILogger() = default;
    virtual void LogInfo(const std::string& message) = 0;
    virtual void LogError(const std::string& message) = 0;
    virtual void LogWarning(const std::string& message) = 0;
};

class ConsoleLogger : public ILogger
{
public:
    void LogInfo(const std::string& message) override;
    void LogError(const std::string& message) override;
    void LogWarning(const std::string& message) override;
};

class EmptyLogger : public ILogger
{
public:
    void LogInfo(const std::string& message) override;
    void LogError(const std::string& message) override;
    void LogWarning(const std::string& message) override;
};
