#include "Debug/Logger.h"

#include <iostream>
#include <cstdio>
#include <chrono>

static std::string ExtractFilename(const char *path)
{
    std::string s(path);

    // Trouver le dernier séparateur
    size_t lastSlash = s.find_last_of("/\\");
    if (lastSlash != std::string::npos)
    {
        s = s.substr(lastSlash + 1);
    }

    // retirer l'extension
    size_t dot = s.rfind('.');
    if (dot != std::string::npos)
    {
        s = s.substr(0, dot);
    }
    return s;
}

// renvoie le temps écoulé depuis l'exécution du code C++ (avant la fenêtre GLFW)
static float GetTimestamp()
{
    static auto startTime = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration<float>(now - startTime).count();
}

std::deque<LogEntry> &Logger::GetEntriesMutable()
{
    // Construit au premier appel, garanti thread-safe
    static std::deque<LogEntry> entries;
    return entries;
}

void Logger::Log(LogLevel level, const char *file, const char *fmt, ...)
{
    // 1. formater le message avec les arguments variadiques
    char buffer[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    // 2. Extraire le nom du fichier propre
    std::string source = ExtractFilename(file);

    // 2. Construire l'entrée de log
    LogEntry entry;
    entry.level = level;
    entry.message = buffer;
    entry.source = source;
    entry.timestamp = GetTimestamp();

    // 3. Fallback console
    switch (level)
    {
    case LogLevel::Trace:
        std::cout << "[Another Engine] [TRACE] [" << source << "] " << buffer << std::endl;
        break;
    case LogLevel::Info:
        std::cout << "[Another Engine] [INFO] [" << source << "] " << buffer << std::endl;
        break;
    case LogLevel::Warning:
        std::cerr << "[Another Engine] [WARN] [" << source << "] " << buffer << std::endl;
        break;
    case LogLevel::Error:
        std::cerr << "[Another Engine] [ERROR] [" << source << "] " << buffer << std::endl;
        break;
    }

    // 4. Ajouter au buffer circulaire
    auto &entries = GetEntriesMutable();
    entries.push_back(std::move(entry));
    if (entries.size() > MAX_ENTRIES)
    {
        entries.pop_front();
    }
}

const std::deque<LogEntry> &Logger::GetEntries()
{
    return GetEntriesMutable();
}

void Logger::Clear()
{
    GetEntriesMutable().clear();
}