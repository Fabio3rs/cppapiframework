#include "../src/jobhandler/JobsHandler.hpp"
#include "../src/jobhandler/QueueWorker.hpp"
#include "../src/jobhandler/QueueableJob.hpp"
#include "../src/queues/StdQueue.hpp"

#include <gtest/gtest.h>

static bool jobrunned = false;
static const std::string queue_name = "test_queue_worker:queue:default";

/**
 * @brief First test job
 *
 */
class MockJob : public job::QueueableJob {
    Poco::JSON::Object::Ptr jsondata;
    std::string strdata;
    int integerdata{0};

    std::string jobresult;

  public:
    QUEUEABLE_SERIALIZE(jsondata, strdata, integerdata)

    auto getName() const -> std::string_view override {
        return getTypeNameByInst(*this);
    }

    void handle() override;

    auto getJobResult() const noexcept -> const std::string & {
        return jobresult;
    }

    MockJob();
    MockJob(const Poco::JSON::Object::Ptr &inputdata,
            const std::string &inputstr, int inputint);
};

MockJob::MockJob() : QueueableJob() {}

MockJob::MockJob(const Poco::JSON::Object::Ptr &inputdata,
                 const std::string &inputstr, int inputint)
    : QueueableJob(), jsondata(inputdata), strdata(inputstr),
      integerdata(inputint) {}

void MockJob::handle() {
    jobrunned = true;
    jsondata->stringify(std::cout, 5);
    std::cout << std::endl;

    std::stringstream sstr;
    jsondata->stringify(sstr, 5);

    sstr << std::endl;
    sstr << strdata;
    sstr << std::endl;
    sstr << integerdata;

    jobresult = sstr.str();
}

/**
 * @brief Other test job
 *
 */
class OtherPrintJob : public job::QueueableJob {
    Poco::JSON::Object::Ptr data;

  public:
    QUEUEABLE_SERIALIZE(data, shouldfail)

    auto getName() const -> std::string_view override {
        return getTypeNameByInst(*this);
    }

    bool shouldfail{false};

    void handle() override;

    OtherPrintJob();
    OtherPrintJob(const Poco::JSON::Object::Ptr &inputdata);
};

OtherPrintJob::OtherPrintJob() : QueueableJob() {}

OtherPrintJob::OtherPrintJob(const Poco::JSON::Object::Ptr &inputdata)
    : QueueableJob(), data(inputdata) {}

void OtherPrintJob::handle() {
    jobrunned = true;
    if (data) {
        data->stringify(std::cout, 5);
        std::cout << std::endl;
    } else {
        std::cout << "PrintJob data is null" << std::endl;
    }

    if (shouldfail) {
        throw std::runtime_error("Failed by something");
    }
}

/**
 * @brief The tests below
 *
 */

TEST(TestJobHandler, AddJobClassToKnownJobsOk) {
    job::JobsHandler handler;
    EXPECT_FALSE(handler.is_job_registered<MockJob>());

    handler.register_job_handler<MockJob>();

    EXPECT_TRUE(handler.is_job_registered<MockJob>());
}

TEST(TestJobHandler, AddRepeatedJobThrows) {
    job::JobsHandler handler;
    EXPECT_FALSE(handler.is_job_registered<MockJob>());

    handler.register_job_handler<MockJob>();

    EXPECT_TRUE(handler.is_job_registered<MockJob>());
    EXPECT_ANY_THROW(handler.register_job_handler<MockJob>());
}

TEST(TestJobHandler, AddDifferentJobNoThrows) {
    job::JobsHandler handler;
    EXPECT_FALSE(handler.is_job_registered<MockJob>());
    EXPECT_FALSE(handler.is_job_registered<OtherPrintJob>());

    EXPECT_NO_THROW(handler.register_job_handler<OtherPrintJob>());
    EXPECT_NO_THROW(handler.register_job_handler<MockJob>());

    EXPECT_TRUE(handler.is_job_registered<MockJob>());
    EXPECT_TRUE(handler.is_job_registered<OtherPrintJob>());
}

TEST(TestJobHandler, JobPayloadHasItClassName) {
    job::JobsHandler handler;
    EXPECT_FALSE(handler.is_job_registered<MockJob>());
    EXPECT_FALSE(handler.is_job_registered<OtherPrintJob>());

    MockJob mockjob;

    auto payload = handler.create_jobpayload(mockjob);
    const std::string className = payload->getValue<std::string>("className");

    EXPECT_EQ(className, job::JobsHandler::getTypeName<MockJob>());
    EXPECT_NE(className, job::JobsHandler::getTypeName<OtherPrintJob>());
}

TEST(TestJobHandler, JobPayloadReinstanceCorrectDynCast) {
    job::JobsHandler handler;

    EXPECT_NO_THROW(handler.register_job_handler<OtherPrintJob>());
    EXPECT_NO_THROW(handler.register_job_handler<MockJob>());

    Poco::JSON::Object::Ptr payload;

    {
        MockJob mockjob;
        payload = handler.create_jobpayload(mockjob);
    }

    {
        auto jobinstance = handler.instance_from_payload(payload);

        EXPECT_NE(jobinstance, nullptr);
        EXPECT_NE(std::dynamic_pointer_cast<MockJob>(jobinstance), nullptr);
    }
}

TEST(TestJobHandler, TestJobDataIntegrity) {
    job::JobsHandler handler;

    EXPECT_NO_THROW(handler.register_job_handler<OtherPrintJob>());
    EXPECT_NO_THROW(handler.register_job_handler<MockJob>());

    Poco::JSON::Object::Ptr payload;

    std::string generatedstr;

    {
        Poco::JSON::Object::Ptr tmpjson(new Poco::JSON::Object);
        tmpjson->set("data1", 1);
        tmpjson->set("str", "this is a string");
        tmpjson->set("boolval", true);

        MockJob mockjob(tmpjson, "test string asdfghjkl", 123456789);
        payload = handler.create_jobpayload(mockjob);

        EXPECT_TRUE(mockjob.getJobResult().empty());
        mockjob.handle();
        EXPECT_FALSE(mockjob.getJobResult().empty());

        generatedstr = mockjob.getJobResult();
    }

    {
        auto jobinstance = handler.instance_from_payload(payload);

        EXPECT_NE(jobinstance, nullptr);

        auto mockjobptr = std::dynamic_pointer_cast<MockJob>(jobinstance);

        EXPECT_NE(mockjobptr, nullptr);

        EXPECT_TRUE(mockjobptr->getJobResult().empty());
        jobinstance->handle();
        EXPECT_FALSE(mockjobptr->getJobResult().empty());

        EXPECT_EQ(generatedstr, mockjobptr->getJobResult());
    }
}

TEST(TestJobQueues, NewQueueIsEmptyDoOneIsFalse) {
    std::shared_ptr<job::JobsHandler> handler(
        std::make_shared<job::JobsHandler>());

    EXPECT_NO_THROW(handler->register_job_handler<OtherPrintJob>());
    EXPECT_NO_THROW(handler->register_job_handler<MockJob>());

    std::shared_ptr<StdQueue> nqueue(std::make_shared<StdQueue>());

    EXPECT_EQ(nqueue->getNumQueues(), 0);

    job::QueueWorker queuew(handler, nqueue);

    EXPECT_FALSE(queuew.do_one(queue_name));
    EXPECT_EQ(nqueue->getQueueSize(queue_name), 0);
}

TEST(TestJobQueues, AddToTheQueueProcessRunOne) {
    std::shared_ptr<job::JobsHandler> handler(
        std::make_shared<job::JobsHandler>());

    EXPECT_NO_THROW(handler->register_job_handler<OtherPrintJob>());
    EXPECT_NO_THROW(handler->register_job_handler<MockJob>());

    std::shared_ptr<StdQueue> nqueue(std::make_shared<StdQueue>());

    EXPECT_EQ(nqueue->getNumQueues(), 0);

    job::QueueWorker queuew(handler, nqueue);

    {
        OtherPrintJob anyjob;
        queuew.push(queue_name, handler->create_jobpayload(anyjob));
        EXPECT_EQ(nqueue->getNumQueues(), 1);
        EXPECT_EQ(nqueue->getQueueSize(queue_name), 1);
    }

    {
        EXPECT_FALSE(jobrunned);
        EXPECT_TRUE(queuew.do_one(queue_name));
        EXPECT_TRUE(jobrunned);

        EXPECT_EQ(nqueue->getQueueSize(queue_name), 0);
    }
}

TEST(TestJobQueues, AddToTheQueueProcessRunOneAndFail) {
    std::shared_ptr<job::JobsHandler> handler(
        std::make_shared<job::JobsHandler>());

    EXPECT_NO_THROW(handler->register_job_handler<OtherPrintJob>());
    EXPECT_NO_THROW(handler->register_job_handler<MockJob>());

    std::shared_ptr<StdQueue> nqueue(std::make_shared<StdQueue>());

    EXPECT_EQ(nqueue->getNumQueues(), 0);

    job::QueueWorker queuew(handler, nqueue);

    {
        OtherPrintJob anyjob;

        anyjob.shouldfail = true; /// Force fail flag

        queuew.push(queue_name, handler->create_jobpayload(anyjob));
        EXPECT_EQ(nqueue->getNumQueues(), 1);
        EXPECT_EQ(nqueue->getQueueSize(queue_name), 1);
    }

    {
        EXPECT_FALSE(jobrunned);
        EXPECT_TRUE(queuew.do_one(queue_name));
        EXPECT_TRUE(jobrunned);

        EXPECT_EQ(nqueue->getQueueSize(queue_name), 1); // Job re-added
    }
}

TEST(TestJobQueues, AddToTheQueueProcessRunMultipleAllFail) {
    std::shared_ptr<job::JobsHandler> handler(
        std::make_shared<job::JobsHandler>());

    EXPECT_NO_THROW(handler->register_job_handler<OtherPrintJob>());
    EXPECT_NO_THROW(handler->register_job_handler<MockJob>());

    std::shared_ptr<StdQueue> nqueue(std::make_shared<StdQueue>());

    EXPECT_EQ(nqueue->getNumQueues(), 0);

    job::QueueWorker queuew(handler, nqueue);

    const size_t MAX_TRIES = 3;

    {
        OtherPrintJob anyjob;

        anyjob.shouldfail = true; /// Force fail flag

        anyjob.setMaxTries(MAX_TRIES);

        queuew.push(queue_name, handler->create_jobpayload(anyjob));
        EXPECT_EQ(nqueue->getNumQueues(), 1);
        EXPECT_EQ(nqueue->getQueueSize(queue_name), 1);
    }

    EXPECT_FALSE(jobrunned);

    for (size_t i = 0; i < MAX_TRIES; i++) {
        EXPECT_TRUE(queuew.do_one(queue_name));
        EXPECT_TRUE(jobrunned);

        EXPECT_EQ(nqueue->getQueueSize(queue_name),
                  i != (MAX_TRIES - 1)); // Job re-added
    }

    EXPECT_EQ(nqueue->getQueueSize(queue_name), 0); // Queue empty
}

TEST(Nothing, CheckNothing) { EXPECT_TRUE(true); }
