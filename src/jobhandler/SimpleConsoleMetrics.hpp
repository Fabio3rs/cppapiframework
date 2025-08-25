/**
 * @file SimpleConsoleMetrics.hpp
 * @author Fabio Rossini Sluzala
 * @brief Implementação simples de métricas que imprime no console
 * @version 0.1
 * @date 2024-12-19
 *
 * @copyright Copyright (c) 2024
 *
 */
#pragma once

#include "WorkerMetricsCallback.hpp"
#include <chrono>
#include <iomanip>
#include <iostream>

namespace job {

/**
 * @brief Implementação simples de métricas que imprime informações no console
 *
 * Esta classe serve como exemplo de como implementar a interface de métricas.
 * Em uma implementação real, você poderia enviar as métricas para sistemas
 * como Prometheus, StatsD, ou bancos de dados.
 */
class SimpleConsoleMetrics : public WorkerMetricsCallback {
  public:
    ~SimpleConsoleMetrics() override;
    
    void onJobQueued(const std::string &queue, const std::string &jobName,
                     const std::string &jobUuid) override {
        std::cout << "[METRICS] Job enfileirado - Queue: " << queue
                  << ", Job: " << jobName << ", UUID: " << jobUuid << std::endl;
    }

    auto onJobStarted(const std::string &queue, const std::string &jobName,
                      const std::string &jobUuid, size_t tries)
        -> std::chrono::steady_clock::time_point override {
        auto startTime = std::chrono::steady_clock::now();

        std::cout << "[METRICS] Job iniciado - Queue: " << queue
                  << ", Job: " << jobName << ", UUID: " << jobUuid
                  << ", Tentativa: " << tries << std::endl;

        return startTime;
    }

    void onJobCompleted(const std::string &queue, const std::string &jobName,
                        const std::string &jobUuid, jobStatus result,
                        std::chrono::steady_clock::time_point startTime,
                        size_t tries) override {
        auto endTime = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            endTime - startTime);

        const char *statusStr = "";
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch-default"
        switch (result) {
        case noerror:
            statusStr = "SUCESSO";
            break;
        case errorremove:
            statusStr = "ERRO_PERMANENTE";
            break;
        case errorretry:
            statusStr = "ERRO_RETRY";
            break;
        case errexcept:
            statusStr = "EXCEÇÃO";
            break;
        }
#pragma clang diagnostic pop

        std::cout << "[METRICS] Job concluído - Queue: " << queue
                  << ", Job: " << jobName << ", UUID: " << jobUuid
                  << ", Status: " << statusStr
                  << ", Duração: " << duration.count() << "ms"
                  << ", Tentativas: " << tries << std::endl;
    }

    void onJobRetry(const std::string &queue, const std::string &jobName,
                    const std::string &jobUuid, size_t tries,
                    int64_t retryAfterSecs) override {
        std::cout << "[METRICS] Job reagendado - Queue: " << queue
                  << ", Job: " << jobName << ", UUID: " << jobUuid
                  << ", Tentativas: " << tries
                  << ", Retry em: " << retryAfterSecs << "s" << std::endl;
    }

    void onJobRemoved(const std::string &queue, const std::string &jobName,
                      const std::string &jobUuid, jobStatus finalResult,
                      size_t totalTries) override {
        const char *statusStr = "";
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch-default"
        switch (finalResult) {
        case noerror:
            statusStr = "SUCESSO";
            break;
        case errorremove:
            statusStr = "ERRO_PERMANENTE";
            break;
        case errorretry:
            statusStr = "ERRO_RETRY";
            break;
        case errexcept:
            statusStr = "EXCEÇÃO";
            break;
        }
#pragma clang diagnostic pop

        std::cout << "[METRICS] Job removido permanentemente - Queue: " << queue
                  << ", Job: " << jobName << ", UUID: " << jobUuid
                  << ", Status final: " << statusStr
                  << ", Total de tentativas: " << totalTries << std::endl;
    }
};

} // namespace job
