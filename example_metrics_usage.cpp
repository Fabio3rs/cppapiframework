/**
 * @file example_metrics_usage.cpp
 * @author Fabio Rossini Sluzala
 * @brief Exemplo de uso das métricas do worker
 * @version 0.1
 * @date 2024-12-19
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "projstdafx.hpp"
#include "jobhandler/WorkerMetricsCallback.hpp"
#include "jobhandler/SimpleConsoleMetrics.hpp"

// Job simples para exemplo
class ExampleJob : public job::QueueableJob {
    std::string message;
    int processTime;

  public:
    QUEUEABLE_SERIALIZE(message, processTime)

    [[nodiscard]] auto getName() const -> std::string override {
        return getTypeNameByInst(*this);
    }

    void handle() override {
        if (processTime > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(processTime));
        }
        
        std::cout << "Processing job: " << message << std::endl;
        
        // Simular possível falha
        if (message == "FAIL") {
            throw std::runtime_error("Job foi programado para falhar");
        }
    }

    ExampleJob() = default;
    ExampleJob(std::string msg, int timeMs) : message(std::move(msg)), processTime(timeMs) {}
};

// Implementação de métricas personalizada que coleta estatísticas
class StatisticsMetrics : public job::WorkerMetricsCallback {
    mutable std::mutex statsMutex;
    std::unordered_map<std::string, size_t> jobCounts;
    std::unordered_map<std::string, std::vector<std::chrono::milliseconds>> durations;
    size_t totalJobs{0};
    size_t successfulJobs{0};
    size_t failedJobs{0};
    size_t retryJobs{0};

  public:
    void onJobQueued(const std::string &queue,
                     const std::string &jobName,
                     const std::string &jobUuid) override {
        std::lock_guard<std::mutex> lock(statsMutex);
        ++totalJobs;
        ++jobCounts[jobName];
        
        std::cout << "[STATS] Job " << jobName << " enfileirado. Total: " << totalJobs << std::endl;
    }

    auto onJobStarted(const std::string &queue,
                      const std::string &jobName,
                      const std::string &jobUuid,
                      size_t tries) -> std::chrono::steady_clock::time_point override {
        std::cout << "[STATS] Iniciando " << jobName << " (tentativa " << tries << ")" << std::endl;
        return std::chrono::steady_clock::now();
    }

    void onJobCompleted(const std::string &queue,
                        const std::string &jobName,
                        const std::string &jobUuid,
                        job::jobStatus result,
                        std::chrono::steady_clock::time_point startTime,
                        size_t tries) override {
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - startTime);

        std::lock_guard<std::mutex> lock(statsMutex);
        durations[jobName].push_back(duration);
        
        if (result == job::noerror) {
            ++successfulJobs;
            std::cout << "[STATS] Job " << jobName << " concluído com sucesso em " 
                     << duration.count() << "ms" << std::endl;
        } else {
            ++failedJobs;
            std::cout << "[STATS] Job " << jobName << " falhou em " 
                     << duration.count() << "ms" << std::endl;
        }
    }

    void onJobRetry(const std::string &queue,
                    const std::string &jobName,
                    const std::string &jobUuid,
                    size_t tries,
                    int64_t retryAfterSecs) override {
        std::lock_guard<std::mutex> lock(statsMutex);
        ++retryJobs;
        
        std::cout << "[STATS] Job " << jobName << " reagendado (tentativa " << tries 
                 << ", próxima em " << retryAfterSecs << "s)" << std::endl;
    }

    void printStatistics() const {
        std::lock_guard<std::mutex> lock(statsMutex);
        
        std::cout << "\n=== ESTATÍSTICAS ===\n";
        std::cout << "Total de jobs: " << totalJobs << std::endl;
        std::cout << "Jobs bem-sucedidos: " << successfulJobs << std::endl;
        std::cout << "Jobs falharam: " << failedJobs << std::endl;
        std::cout << "Jobs reagendados: " << retryJobs << std::endl;
        
        std::cout << "\nTempo médio por tipo de job:\n";
        for (const auto& [jobName, times] : durations) {
            if (!times.empty()) {
                auto total = std::accumulate(times.begin(), times.end(), 
                                           std::chrono::milliseconds{0});
                auto avg = total / times.size();
                std::cout << "  " << jobName << ": " << avg.count() << "ms (média de " 
                         << times.size() << " execuções)" << std::endl;
            }
        }
        std::cout << "===================\n\n";
    }
};

int main() {
    try {
        // Configurar o handler de jobs
        auto handler = std::make_shared<job::JobsHandler>();
        handler->register_job_handler<ExampleJob>();
        
        // Configurar a fila (usando StdQueue para simplicidade)
        auto queue = std::make_shared<StdQueue>();
        
        // Criar o worker
        job::QueueWorker worker(handler, queue);
        
        // Criar e configurar as métricas
        auto metrics = std::make_shared<StatisticsMetrics>();
        worker.setMetricsCallback(metrics);
        
        const std::string queueName = "example_queue";
        
        std::cout << "=== DEMONSTRAÇÃO DAS MÉTRICAS DO WORKER ===\n\n";
        
        // Adicionar alguns jobs de exemplo
        std::cout << "Adicionando jobs à fila...\n";
        worker.push(queueName, ExampleJob("Processamento rápido", 50));
        worker.push(queueName, ExampleJob("Processamento médio", 200));
        worker.push(queueName, ExampleJob("FAIL", 100)); // Este job irá falhar
        worker.push(queueName, ExampleJob("Processamento longo", 500));
        
        std::cout << "\nProcessando jobs...\n";
        
        // Processar todos os jobs na fila
        int processedJobs = 0;
        while (worker.do_one(queueName)) {
            ++processedJobs;
            if (processedJobs > 10) { // Evitar loop infinito em caso de retry
                std::cout << "Limite de processamento atingido\n";
                break;
            }
        }
        
        std::cout << "\nTodos os jobs foram processados!\n";
        
        // Mostrar estatísticas finais
        metrics->printStatistics();
        
        std::cout << "=== EXEMPLO CONCLUÍDO ===\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Erro: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}