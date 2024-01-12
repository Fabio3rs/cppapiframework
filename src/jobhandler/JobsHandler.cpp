#include "../projstdafx.hpp"

auto job::JobsHandler::default_instance() -> std::shared_ptr<JobsHandler> {
    static std::shared_ptr<JobsHandler> instance(
        std::make_shared<JobsHandler>());
    return instance;
}

auto job::JobsHandler::handle(const std::shared_ptr<QueueableJob> &newjob,
                              std::ostream &outstream, std::ostream &errstream)
    -> jobStatus {
    jobStatus result = noerror;
    ScopedStreamRedirect red(std::cout, outstream);
    ScopedStreamRedirect redcerr(std::cerr, errstream);

    try {
        newjob->handle();
    } catch (const Poco::Exception &e) {
        std::cerr << __FILE__ << "@" << __LINE__
                  << ": Job exception: " << e.what() << "; " << e.displayText()
                  << '\n';
        result = errexcept;
    } catch (const std::exception &e) {
        std::cerr << __FILE__ << "@" << __LINE__
                  << ": Job exception: " << e.what() << '\n';
        result = errexcept;
    }

    std::cout.flush();
    std::cerr.flush();
    return result;
}

static auto putLogInDataMap(std::fstream &joblog, const std::string &key,
                            job::JobsHandler::datamap_t &datamap) {
    std::string line;
    std::string fullstrlog;
    joblog.seekg(0, std::ios::end);

    std::streampos printsize = joblog.tellg();
    fullstrlog.reserve((printsize > 0 ? static_cast<size_t>(printsize) : 0) +
                       8);
    joblog.seekg(0, std::ios::beg);

    while (std::getline(joblog, line)) {
        std::cout << "JOB LINE " << line << std::endl;
        fullstrlog += line;
        fullstrlog += "\n";
    }

    datamap[key] = fullstrlog;
}

void job::JobsHandler::saveJobLog(std::fstream &outstream,
                                  std::fstream &errstream, datamap_t &datamap) {
    try {
        putLogInDataMap(errstream, "JobStderr", datamap);
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        datamap["LastException"] = e.what();
    }

    try {
        putLogInDataMap(outstream, "JobStdout", datamap);
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        datamap["LastException"] = e.what();
    }
}

job::JobsHandler::~JobsHandler() = default;
