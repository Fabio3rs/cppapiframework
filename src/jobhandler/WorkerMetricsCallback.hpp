/**
 * @file WorkerMetricsCallback.hpp
 * @author Fabio Rossini Sluzala
 * @brief Interface para callback de métricas do worker de jobs
 * @version 0.1
 * @date 2024-12-19
 *
 * @copyright Copyright (c) 2024
 *
 */
#pragma once

#include "../stdafx.hpp"
#include "JobsHandler.hpp"
#include <chrono>
#include <string>

namespace job {

/**
 * @brief Interface para callback de métricas do QueueWorker
 *
 * Esta interface permite capturar métricas durante o ciclo de vida dos jobs
 * sem alterar a lógica principal do processamento.
 */
class WorkerMetricsCallback {
  public:
    virtual ~WorkerMetricsCallback() = default;

    /**
     * @brief Chamado quando um job é adicionado à fila
     *
     * @param queue nome da fila
     * @param jobName nome da classe do job
     * @param jobUuid UUID único do job
     */
    virtual void onJobQueued(const std::string &queue,
                             const std::string &jobName,
                             const std::string &jobUuid) {}

    /**
     * @brief Chamado quando o processamento de um job inicia
     *
     * @param queue nome da fila
     * @param jobName nome da classe do job
     * @param jobUuid UUID único do job
     * @param tries número atual de tentativas
     * @return std::chrono::steady_clock::time_point timestamp de início
     */
    virtual auto
    onJobStarted(const std::string &queue, const std::string &jobName,
                 const std::string &jobUuid,
                 size_t tries) -> std::chrono::steady_clock::time_point {
        (void)queue;
        (void)jobName;
        (void)jobUuid;
        (void)tries;
        return std::chrono::steady_clock::now();
    }

    /**
     * @brief Chamado quando um job termina (sucesso ou falha)
     *
     * @param queue nome da fila
     * @param jobName nome da classe do job
     * @param jobUuid UUID único do job
     * @param result resultado do job
     * @param startTime timestamp de início (retornado por onJobStarted)
     * @param tries número final de tentativas
     */
    virtual void onJobCompleted(const std::string &queue,
                                const std::string &jobName,
                                const std::string &jobUuid, jobStatus result,
                                std::chrono::steady_clock::time_point startTime,
                                size_t tries) {
        (void)queue;
        (void)jobName;
        (void)jobUuid;
        (void)result;
        (void)startTime;
        (void)tries;
    }

    /**
     * @brief Chamado quando um job é reprocessado (retry)
     *
     * @param queue nome da fila
     * @param jobName nome da classe do job
     * @param jobUuid UUID único do job
     * @param tries número atual de tentativas
     * @param retryAfterSecs segundos para retry (0 se imediato)
     */
    virtual void onJobRetry(const std::string &queue,
                            const std::string &jobName,
                            const std::string &jobUuid, size_t tries,
                            int64_t retryAfterSecs) {
        (void)queue;
        (void)jobName;
        (void)jobUuid;
        (void)tries;
        (void)retryAfterSecs;
    }

    /**
     * @brief Chamado quando um job é removido permanentemente da fila
     *
     * @param queue nome da fila
     * @param jobName nome da classe do job
     * @param jobUuid UUID único do job
     * @param finalResult resultado final do job
     * @param totalTries número total de tentativas realizadas
     */
    virtual void onJobRemoved(const std::string &queue,
                              const std::string &jobName,
                              const std::string &jobUuid, jobStatus finalResult,
                              size_t totalTries) {
        (void)queue;
        (void)jobName;
        (void)jobUuid;
        (void)finalResult;
        (void)totalTries;
    }
};

} // namespace job
