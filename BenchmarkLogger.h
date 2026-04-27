#pragma once

// ============================================================
//  BenchmarkLogger
//
//  Простой singleton для сбора замеров производительности
//  и метрик деформации. Все замеры пишутся в std::vector
//  в памяти, при вызове Flush() — сбрасываются в CSV-файл.
//
//  Использование:
//      auto t = BenchmarkLogger::Instance().BeginScope("ApplyDent");
//      // ... код ...
//      // t уничтожится в конце scope, время запишется
//
//      BenchmarkLogger::Instance().LogValue("CraterDepth", 12.5);
//
//      BenchmarkLogger::Instance().Flush("benchmark.csv");
// ============================================================

#include <chrono>
#include <string>
#include <vector>
#include <fstream>
#include <mutex>

class BenchmarkLogger
{
public:
    struct Entry
    {
        std::string category;   // "ApplyDent_ms", "UploadToGPU_ms", "CraterDepth", ...
        double      value;      // время в мс / глубина / любая числовая метрика
        int         hitIndex;   // номер удара (1, 2, 3, ...) или -1 если неприменимо
    };

    static BenchmarkLogger& Instance()
    {
        static BenchmarkLogger inst;
        return inst;
    }

    // RAII-таймер: создаём в начале scope, при разрушении пишет результат
    class ScopedTimer
    {
    public:
        ScopedTimer(const std::string& category)
            : category_(category), start_(std::chrono::high_resolution_clock::now())
        {
        }
        ~ScopedTimer()
        {
            auto end = std::chrono::high_resolution_clock::now();
            double ms = std::chrono::duration<double, std::milli>(end - start_).count();
            BenchmarkLogger::Instance().LogValue(category_, ms);
        }
    private:
        std::string                                                category_;
        std::chrono::time_point<std::chrono::high_resolution_clock> start_;
    };

    // Залогировать произвольное значение
    void LogValue(const std::string& category, double value)
    {
        std::lock_guard<std::mutex> lk(mtx_);
        entries_.push_back({ category, value, currentHitIndex_ });
    }

    // Установить «номер текущего удара». Все последующие LogValue до
    // следующего вызова получат этот номер. -1 = вне удара.
    void SetCurrentHitIndex(int idx)
    {
        std::lock_guard<std::mutex> lk(mtx_);
        currentHitIndex_ = idx;
    }

    int GetCurrentHitIndex() const
    {
        return currentHitIndex_;
    }

    // Сбросить все замеры в CSV. Безопасно вызывать многократно
    void Flush(const std::string& path)
    {
        std::lock_guard<std::mutex> lk(mtx_);
        std::ofstream out(path);
        if (!out.is_open()) return;
        out << "category;hit_index;value\n";
        for (const auto& e : entries_)
            out << e.category << ";" << e.hitIndex << ";" << e.value << "\n";
        out.flush();
    }

    // Очистить накопленные замеры (например, между запусками)
    void Clear()
    {
        std::lock_guard<std::mutex> lk(mtx_);
        entries_.clear();
        currentHitIndex_ = -1;
    }

    size_t Size() const { return entries_.size(); }

private:
    BenchmarkLogger() = default;

    mutable std::mutex mtx_;
    std::vector<Entry> entries_;
    int                currentHitIndex_ = -1;
};

// Удобный макрос для замера времени в текущем scope
#define BENCH_SCOPE(name) BenchmarkLogger::ScopedTimer _bench_timer_##__LINE__(name)