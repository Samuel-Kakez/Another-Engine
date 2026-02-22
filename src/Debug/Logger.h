#pragma once

#include <string>
#include <deque>
#include <cstdarg>

/// @brief Niveaux de sévérité pour les messages de log
enum class LogLevel
{
    Trace,
    Info,
    Warning,
    Error
};

/// @brief Représente une entrée de log individuelle
struct LogEntry
{
    LogLevel level;
    std::string message;
    std::string source; // nom du fichier source
    float timestamp; // secondes depuis le démarrage du moteur
};

/// @brief Classe statique de logging, accessible depuis n'importe où dans le moteur
/// @details Stocke les messages dans un buffer circulaire (deque) et les affiche également
/// dans la console standard en fallback
class Logger
{
public:
    static constexpr size_t MAX_ENTRIES = 1000;

    /// @brief Enregistre un message formaté avec un niveau de sévérité
    /// @param level niveau de sévérité du message
    /// @param fmt chaîne de format printf
    /// @param  ... arguments variadiques pour le formattage
    static void Log(LogLevel level, const char* file, const char *fmt, ...);

    /// @brief Retourne une ref constante vers le buffer de logs
    /// @return const std::deque<LogEntry>& le buffer circulaire de logs
    static const std::deque<LogEntry> &GetEntries();

    /// @brief Vide le buffer de logs
    static void Clear();

private:
    static std::deque<LogEntry>& GetEntriesMutable();
};

/// --- Macros ---
#define LOG_TRACE(...) Logger::Log(LogLevel::Trace,__FILE__,  __VA_ARGS__)
#define LOG_INFO(...) Logger::Log(LogLevel::Info, __FILE__,  __VA_ARGS__)
#define LOG_WARN(...) Logger::Log(LogLevel::Warning, __FILE__,  __VA_ARGS__)
#define LOG_ERROR(...) Logger::Log(LogLevel::Error, __FILE__,  __VA_ARGS__)