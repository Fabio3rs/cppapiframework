# Integração de Métricas do Worker

Este documento descreve a integração de métricas no sistema de jobs do cppapiframework através de callbacks.

## Visão Geral

A funcionalidade de métricas permite capturar informações detalhadas sobre o processamento de jobs sem alterar a lógica principal do sistema. A integração é feita através de callbacks opcionais que são chamados em pontos específicos do ciclo de vida dos jobs.

## Interface de Callback

A interface `WorkerMetricsCallback` define os métodos que podem ser implementados para capturar métricas:

```cpp
class WorkerMetricsCallback {
public:
    // Chamado quando um job é adicionado à fila
    virtual void onJobQueued(const std::string &queue,
                             const std::string &jobName,
                             const std::string &jobUuid);

    // Chamado quando o processamento de um job inicia
    virtual auto onJobStarted(const std::string &queue,
                              const std::string &jobName,
                              const std::string &jobUuid,
                              size_t tries) -> std::chrono::steady_clock::time_point;

    // Chamado quando um job termina (sucesso ou falha)
    virtual void onJobCompleted(const std::string &queue,
                                const std::string &jobName,
                                const std::string &jobUuid,
                                jobStatus result,
                                std::chrono::steady_clock::time_point startTime,
                                size_t tries);

    // Chamado quando um job é reprocessado (retry)
    virtual void onJobRetry(const std::string &queue,
                            const std::string &jobName,
                            const std::string &jobUuid,
                            size_t tries,
                            int64_t retryAfterSecs);

    // Chamado quando um job é removido permanentemente da fila
    virtual void onJobRemoved(const std::string &queue,
                              const std::string &jobName,
                              const std::string &jobUuid,
                              jobStatus finalResult,
                              size_t totalTries);
};
```

## Como Usar

### 1. Configurar o Callback

```cpp
// Criar o worker
job::QueueWorker worker(handler, queue);

// Criar e configurar as métricas
auto metrics = std::make_shared<MinhaImplementacaoMetricas>();
worker.setMetricsCallback(metrics);
```

### 2. Implementações Disponíveis

#### SimpleConsoleMetrics
Uma implementação simples que imprime as métricas no console:

```cpp
#include "jobhandler/SimpleConsoleMetrics.hpp"

auto consoleMetrics = std::make_shared<job::SimpleConsoleMetrics>();
worker.setMetricsCallback(consoleMetrics);
```

#### Implementação Personalizada
Você pode criar sua própria implementação:

```cpp
class MinhaMetricas : public job::WorkerMetricsCallback {
public:
    void onJobQueued(const std::string &queue,
                     const std::string &jobName,
                     const std::string &jobUuid) override {
        // Enviar métrica para Prometheus, StatsD, etc.
        prometheus_counter_inc("jobs_queued_total", {{"queue", queue}, {"job", jobName}});
    }

    void onJobCompleted(const std::string &queue,
                        const std::string &jobName,
                        const std::string &jobUuid,
                        job::jobStatus result,
                        std::chrono::steady_clock::time_point startTime,
                        size_t tries) override {
        auto duration = std::chrono::steady_clock::now() - startTime;
        auto durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
        
        // Registrar tempo de execução
        prometheus_histogram_observe("job_duration_milliseconds", 
                                   durationMs.count(), 
                                   {{"queue", queue}, {"job", jobName}});
        
        // Registrar resultado
        std::string status = (result == job::noerror) ? "success" : "failure";
        prometheus_counter_inc("jobs_completed_total", 
                             {{"queue", queue}, {"job", jobName}, {"status", status}});
    }
};
```

## Métricas Capturadas

### Informações Básicas
- **queue**: Nome da fila onde o job está sendo processado
- **jobName**: Nome da classe do job (tipo)
- **jobUuid**: Identificador único do job
- **tries**: Número de tentativas de execução

### Eventos do Ciclo de Vida
1. **onJobQueued**: Job foi adicionado à fila
2. **onJobStarted**: Processamento do job iniciou
3. **onJobCompleted**: Job terminou (com sucesso ou falha)
4. **onJobRetry**: Job será reprocessado devido a falha
5. **onJobRemoved**: Job foi removido permanentemente da fila

### Status de Resultado
- `noerror`: Job executado com sucesso
- `errorretry`: Job falhou mas será reagendado
- `errorremove`: Job falhou e foi removido permanentemente
- `errexcept`: Job lançou exceção não capturada

## Exemplos de Uso

### Métricas para Prometheus
```cpp
class PrometheusMetrics : public job::WorkerMetricsCallback {
    prometheus::Counter& jobs_queued;
    prometheus::Counter& jobs_completed;
    prometheus::Histogram& job_duration;
    prometheus::Counter& jobs_retried;

public:
    PrometheusMetrics(/* configurar contadores */) { }
    
    void onJobQueued(const std::string &queue, const std::string &jobName, 
                     const std::string &jobUuid) override {
        jobs_queued.Increment({{"queue", queue}, {"job_type", jobName}});
    }
    
    // ... implementar outros métodos
};
```

### Logging Estruturado
```cpp
class StructuredLogMetrics : public job::WorkerMetricsCallback {
public:
    void onJobCompleted(const std::string &queue, const std::string &jobName,
                        const std::string &jobUuid, job::jobStatus result,
                        std::chrono::steady_clock::time_point startTime,
                        size_t tries) override {
        auto duration = std::chrono::steady_clock::now() - startTime;
        auto durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
        
        json logEntry = {
            {"event", "job_completed"},
            {"timestamp", std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch()).count()},
            {"queue", queue},
            {"job_type", jobName},
            {"job_uuid", jobUuid},
            {"duration_ms", durationMs.count()},
            {"tries", tries},
            {"success", result == job::noerror}
        };
        
        std::cout << logEntry.dump() << std::endl;
    }
};
```

## Compatibilidade

- **Retrocompatibilidade**: A funcionalidade é completamente opcional. Código existente continuará funcionando sem modificações.
- **Performance**: Os callbacks são chamados apenas se configurados, não há overhead quando não utilizados.
- **Thread Safety**: As implementações de métricas devem ser thread-safe se o worker for usado em ambiente multi-thread.

## Compilação

Os arquivos de métricas são incluídos automaticamente na build. Não é necessário modificar o CMakeLists.txt.

Arquivos adicionados:
- `src/jobhandler/WorkerMetricsCallback.hpp` - Interface base
- `src/jobhandler/SimpleConsoleMetrics.hpp` - Implementação de exemplo
- `tests/test_worker_metrics.cpp` - Testes de validação
- `example_metrics_usage.cpp` - Exemplo de uso completo