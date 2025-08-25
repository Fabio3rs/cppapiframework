#include "projstdafx.hpp"

#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>

// Incluir as classes de métricas
#include "jobhandler/SimpleConsoleMetrics.hpp"

// Implementação simples de callback para testes
class TestMetricsCallback : public job::WorkerMetricsCallback {
  public:
    ~TestMetricsCallback() override;
    
    struct Event {
        std::string type;
        std::string queue;
        std::string jobName;
        std::string jobUuid;
        job::jobStatus status{job::noerror};
        size_t tries{0};
        int64_t retryAfterSecs{0};
        std::chrono::milliseconds duration{0};
    };

    std::vector<Event> events;

    void onJobQueued(const std::string &queue, const std::string &jobName,
                     const std::string &jobUuid) override {
        events.push_back({"QUEUED", queue, jobName, jobUuid});
    }

    auto onJobStarted(const std::string &queue, const std::string &jobName,
                      const std::string &jobUuid, size_t tries)
        -> std::chrono::steady_clock::time_point override {
        auto startTime = std::chrono::steady_clock::now();
        events.push_back(
            {"STARTED", queue, jobName, jobUuid, job::noerror, tries});
        return startTime;
    }

    void onJobCompleted(const std::string &queue, const std::string &jobName,
                        const std::string &jobUuid, job::jobStatus result,
                        std::chrono::steady_clock::time_point startTime,
                        size_t tries) override {
        auto endTime = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            endTime - startTime);
        events.push_back(
            {"COMPLETED", queue, jobName, jobUuid, result, tries, 0, duration});
    }

    void onJobRetry(const std::string &queue, const std::string &jobName,
                    const std::string &jobUuid, size_t tries,
                    int64_t retryAfterSecs) override {
        events.push_back({"RETRY", queue, jobName, jobUuid, job::errorretry,
                          tries, retryAfterSecs});
    }

    void onJobRemoved(const std::string &queue, const std::string &jobName,
                      const std::string &jobUuid, job::jobStatus finalResult,
                      size_t totalTries) override {
        events.push_back(
            {"REMOVED", queue, jobName, jobUuid, finalResult, totalTries});
    }
};

// Test job simples para usar nos testes
class TestMetricsJob : public job::QueueableJob {
    bool shouldFail{false};
    int failAfter{0};

  public:
    ~TestMetricsJob() override;
    
    QUEUEABLE_SERIALIZE(shouldFail, failAfter)

    [[nodiscard]] auto getName() const -> std::string override {
        return getTypeNameByInst(*this);
    }

    void setShouldFail(bool fail) { shouldFail = fail; }
    void setFailAfter(int seconds) { failAfter = seconds; }

    void handle() override {
        if (shouldFail) {
            if (failAfter > 0) {
                setRetryAfter(failAfter);
            }
            throw std::runtime_error("Test job failure");
        }
        // Job de sucesso - não faz nada
    }

    TestMetricsJob() = default;
};

static const std::string test_queue_name = "test_metrics_queue:default";

// NOLINTNEXTLINE
TEST(TestWorkerMetrics, CallbackIsCalledWhenSet) {
    std::shared_ptr<job::JobsHandler> handler(
        std::make_shared<job::JobsHandler>());

    handler->register_job_handler<TestMetricsJob>();

    std::shared_ptr<StdQueue> queue(std::make_shared<StdQueue>());
    job::QueueWorker worker(handler, queue);

    // Criar callback de teste
    auto metricsCallback = std::make_shared<TestMetricsCallback>();
    worker.setMetricsCallback(metricsCallback);

    // Verificar se o callback foi definido corretamente
    EXPECT_EQ(worker.getMetricsCallback(), metricsCallback);

    // Adicionar e processar um job
    TestMetricsJob testJob;
    std::string jobUuid = worker.push(test_queue_name, testJob);

    // Verificar evento de enfileiramento
    EXPECT_EQ(metricsCallback->events.size(), 1);
    EXPECT_EQ(metricsCallback->events[0].type, "QUEUED");
    EXPECT_EQ(metricsCallback->events[0].queue, test_queue_name);
    EXPECT_EQ(metricsCallback->events[0].jobUuid, jobUuid);

    // Processar o job
    EXPECT_TRUE(worker.do_one(test_queue_name));

    // Verificar eventos de início e conclusão
    EXPECT_GE(metricsCallback->events.size(),
              3); // QUEUED + STARTED + COMPLETED

    // Encontrar evento STARTED
    auto startedIt = std::find_if(
        metricsCallback->events.begin(), metricsCallback->events.end(),
        [](const auto &e) { return e.type == "STARTED"; });
    EXPECT_NE(startedIt, metricsCallback->events.end());
    EXPECT_EQ(startedIt->queue, test_queue_name);
    EXPECT_EQ(startedIt->jobUuid, jobUuid);

    // Encontrar evento COMPLETED
    auto completedIt = std::find_if(
        metricsCallback->events.begin(), metricsCallback->events.end(),
        [](const auto &e) { return e.type == "COMPLETED"; });
    EXPECT_NE(completedIt, metricsCallback->events.end());
    EXPECT_EQ(completedIt->queue, test_queue_name);
    EXPECT_EQ(completedIt->jobUuid, jobUuid);
    EXPECT_EQ(completedIt->status, job::noerror);
}

// NOLINTNEXTLINE
TEST(TestWorkerMetrics, NoCallbackDoesNotCrash) {
    std::shared_ptr<job::JobsHandler> handler(
        std::make_shared<job::JobsHandler>());

    handler->register_job_handler<TestMetricsJob>();

    std::shared_ptr<StdQueue> queue(std::make_shared<StdQueue>());
    job::QueueWorker worker(handler, queue);

    // Não definir callback - deve funcionar normalmente
    EXPECT_EQ(worker.getMetricsCallback(), nullptr);

    // Adicionar e processar um job
    TestMetricsJob testJob;
    std::string jobUuid = worker.push(test_queue_name, testJob);

    // Deve processar sem problemas
    EXPECT_TRUE(worker.do_one(test_queue_name));
}

// NOLINTNEXTLINE
TEST(TestWorkerMetrics, RetryJobCallsRetryCallback) {
    std::shared_ptr<job::JobsHandler> handler(
        std::make_shared<job::JobsHandler>());

    handler->register_job_handler<TestMetricsJob>();

    std::shared_ptr<StdQueue> queue(std::make_shared<StdQueue>());
    job::QueueWorker worker(handler, queue);

    auto metricsCallback = std::make_shared<TestMetricsCallback>();
    worker.setMetricsCallback(metricsCallback);

    // Criar job que falha
    TestMetricsJob testJob;
    testJob.setShouldFail(true);
    testJob.setMaxTries(2);

    std::string jobUuid = worker.push(test_queue_name, testJob);

    // Processar primeira vez (deve falhar e ser reagendado)
    EXPECT_TRUE(worker.do_one(test_queue_name));

    // Verificar se teve evento de retry
    auto retryIt = std::find_if(
        metricsCallback->events.begin(), metricsCallback->events.end(),
        [](const auto &e) { return e.type == "RETRY"; });
    EXPECT_NE(retryIt, metricsCallback->events.end());
    EXPECT_EQ(retryIt->queue, test_queue_name);
    EXPECT_EQ(retryIt->jobUuid, jobUuid);
}

// NOLINTNEXTLINE
TEST(TestWorkerMetrics, SimpleConsoleMetricsWorks) {
    // Teste básico da implementação de exemplo
    auto consoleMetrics = std::make_shared<job::SimpleConsoleMetrics>();

    // Simular chamadas de callback
    consoleMetrics->onJobQueued("test_queue", "TestJob", "uuid-123");

    auto startTime =
        consoleMetrics->onJobStarted("test_queue", "TestJob", "uuid-123", 1);

    // Pequena espera para simular processamento
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    consoleMetrics->onJobCompleted("test_queue", "TestJob", "uuid-123",
                                   job::noerror, startTime, 1);

    // Se chegou até aqui sem crash, a implementação básica funciona
    SUCCEED();
}

// Virtual destructor implementations to avoid weak vtables warning
TestMetricsCallback::~TestMetricsCallback() = default;
TestMetricsJob::~TestMetricsJob() = default;
