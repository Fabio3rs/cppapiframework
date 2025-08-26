# CPP API Framework

![Build Status](https://github.com/Fabio3rs/cppapiframework/actions/workflows/Test%2C%20Build%20and%20Test-main.yml/badge.svg)
![CodeQL](https://github.com/Fabio3rs/cppapiframework/actions/workflows/codeql.yml/badge.svg)
![License](https://img.shields.io/github/license/Fabio3rs/cppapiframework)
![C++](https://img.shields.io/badge/C%2B%2B-20-blue.svg)
![Security](https://img.shields.io/badge/Security-Enterprise%20Grade-green.svg)

A modern C++20 framework for building REST APIs with advanced features like job processing, metrics collection, WebSocket support, and **enterprise-grade security scanning**.

## 🚀 Features

### 🛡️ Enterprise Security
- **Static Application Security Testing (SAST)**: CodeQL, enhanced clang-tidy, cppcheck
- **Software Composition Analysis (SCA)**: Dependabot for vulnerability scanning
- **Security-Hardened Build**: Stack protection, PIE, RELRO, and other hardening flags  
- **Input Validation & Sanitization**: SQL injection prevention, comprehensive input validation
- **Memory Safety**: RAII patterns, smart pointers, bounds checking
- **Supply Chain Security**: SBOM generation, dependency monitoring

### Web Framework
- **REST API Development**: Built on top of [Pistache](https://github.com/pistacheio/pistache) HTTP library
- **OpenAPI/Swagger Integration**: Automatic API documentation generation
- **Input Validation**: Comprehensive request validation system
- **WebSocket Support**: Real-time bidirectional communication
- **HTTP Wrappers**: Simplified request/response handling
- **SSL/TLS Support**: Secure communications

### Job Processing System
- **Queueable Jobs**: Asynchronous job processing with serialization support
- **Worker Threads**: Multi-threaded job execution
- **Metrics Collection**: Comprehensive job metrics via callback system
- **Job Retry Logic**: Automatic retry mechanisms for failed jobs
- **Queue Management**: Multiple queue support with priority handling

### Storage & Database
- **MySQL Integration**: Native MySQL C++ Connector support
- **Redis Support**: Caching and session management via Poco Redis
- **Storage Controllers**: File and data storage abstractions
- **Database Migrations**: Schema management utilities

### Development Tools
- **Code Generation**: Automatic job class generation (`newjob.sh`)
- **Testing Framework**: Google Test integration
- **Code Quality**: Clang-tidy and clang-format support
- **Docker Ready**: Containerization support

## ⚠️ Project Status

**Important Notice**: This is a personal project currently under development and is provided without any warranties or guarantees. The framework is experimental and should not be used in production environments without thorough testing and evaluation. Features may change, and compatibility is not guaranteed across versions.

This project serves primarily as a learning exercise and concept exploration for modern C++20 API development patterns.

## 📋 Requirements

### Dependencies
- **Compiler**: GCC 13+ or Clang 18+ (C++20 support required)
- **Build System**: CMake 3.16+
- **Libraries**:
  - [Poco C++ Libraries](https://pocoproject.org/) (JSON, Net, NetSSL, Redis, Crypto)
  - [Pistache HTTP](https://github.com/pistacheio/pistache) 
  - [MySQL C++ Connector](https://dev.mysql.com/downloads/connector/cpp/)
  - [Google Test](https://github.com/google/googletest) (for testing)
  - OpenSSL

### System Requirements
- Linux (Ubuntu 20.04+ recommended)
- Redis Server (optional, for caching features)
- MySQL Server (optional, for database features)

## 🔧 Installation

### Ubuntu/Debian
```bash
# Install build tools
sudo apt-get update && sudo apt-get install build-essential cmake ninja-build

# Install Clang (recommended)
sudo apt install clang-18 clang-tidy-18 clang-format

# Install Pistache
sudo add-apt-repository ppa:pistache+team/unstable
sudo apt update && sudo apt install libpistache-dev

# Install other dependencies  
sudo apt-get install libpoco-dev libmysqlcppconn-dev libgtest-dev
sudo apt-get install redis-server redis-tools  # Optional
```

### Build from Source
```bash
# Clone the repository
git clone https://github.com/Fabio3rs/cppapiframework.git
cd cppapiframework

# Configure build
mkdir build && cd build
export CC=$(which clang-18)    # Optional: use Clang
export CXX=$(which clang++-18)
cmake .. -G Ninja

# Build
cmake --build . --config Debug --target all -j $(nproc)

# Run tests
ctest -j 20 -C Debug -T test --output-on-failure
```

## 🚦 Quick Start

### Creating a Simple API

```cpp
#include "WebInterface/WebApp.hpp"
#include "WebInterface/CController.hpp"

class HelloController : public CController {
public:
    void setupRoutes() override {
        // GET /hello
        addRoute("GET", "/hello", [this](auto req, auto res) {
            Json::Value response;
            response["message"] = "Hello, World!";
            response["timestamp"] = getCurrentTimestamp();
            sendJsonResponse(res, Pistache::Http::Code::Ok, response);
        });
        
        // POST /hello/:name
        addRoute("POST", "/hello/:name", [this](auto req, auto res) {
            auto name = req.param(":name").as<std::string>();
            Json::Value response;
            response["greeting"] = "Hello, " + name + "!";
            sendJsonResponse(res, Pistache::Http::Code::Ok, response);
        });
    }
};

int main() {
    webapp::WebApp app;
    
    // Initialize web server
    app.init(Pistache::Address("localhost", 8080), 4)
       .startAsync();
    
    // Register controller
    auto controller = std::make_shared<HelloController>();
    app.getRouter().addController("/api", controller);
    
    std::cout << "Server running on http://localhost:8080" << std::endl;
    
    // Keep running
    app.start([]() {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    });
    
    return 0;
}
```

### Creating Background Jobs

```cpp
#include "jobhandler/QueueableJob.hpp"
#include "jobhandler/QueueWorker.hpp"

// Define a job class
class EmailJob : public job::QueueableJob {
    std::string recipient;
    std::string subject;
    std::string body;

public:
    QUEUEABLE_SERIALIZE(recipient, subject, body)

    [[nodiscard]] auto getName() const -> std::string override {
        return getTypeNameByInst(*this);
    }

    void handle() override {
        // Send email logic here
        std::cout << "Sending email to: " << recipient << std::endl;
        std::cout << "Subject: " << subject << std::endl;
        
        // Simulate processing time
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    EmailJob() = default;
    EmailJob(std::string to, std::string subj, std::string content)
        : recipient(std::move(to)), subject(std::move(subj)), body(std::move(content)) {}
};

int main() {
    // Create job handler and worker
    auto handler = std::make_shared<job::JobsHandler>();
    auto worker = job::QueueWorker(handler, "email_queue");
    
    // Queue some jobs
    worker.push("email_queue", EmailJob("user@example.com", "Welcome!", "Welcome to our service"));
    worker.push("email_queue", EmailJob("admin@example.com", "Alert", "System status update"));
    
    // Process jobs
    while (worker.hasJobs("email_queue")) {
        worker.do_one("email_queue");
    }
    
    return 0;
}
```

### Job Metrics Collection

```cpp
#include "jobhandler/SimpleConsoleMetrics.hpp"
#include "jobhandler/WorkerMetricsCallback.hpp"

// Custom metrics implementation
class PrometheusMetrics : public job::WorkerMetricsCallback {
public:
    void onJobQueued(const std::string &queue, const std::string &jobName, 
                     const std::string &jobUuid) override {
        // Increment job queued counter
        prometheus_counter_inc("jobs_queued_total", 
                             {{"queue", queue}, {"job_type", jobName}});
    }
    
    void onJobCompleted(const std::string &queue, const std::string &jobName,
                       const std::string &jobUuid, job::JobResult result,
                       const std::chrono::milliseconds &duration) override {
        // Record job duration
        prometheus_histogram_observe("job_duration_seconds", 
                                   duration.count() / 1000.0,
                                   {{"queue", queue}, {"job_type", jobName}});
        
        // Increment completion counter
        std::string status = (result == job::noerror) ? "success" : "failure";
        prometheus_counter_inc("jobs_completed_total",
                             {{"queue", queue}, {"job_type", jobName}, {"status", status}});
    }
};

// Setup metrics
auto metrics = std::make_shared<PrometheusMetrics>();
worker.setMetricsCallback(metrics);
```

## 🏗️ Architecture

### Core Components

```
cppapiframework/
├── src/
│   ├── WebInterface/          # HTTP/REST API components
│   │   ├── WebApp.hpp         # Main web application class
│   │   ├── CController.hpp    # Base controller class
│   │   └── httpwrappers.hpp   # HTTP utilities
│   ├── jobhandler/            # Background job processing
│   │   ├── QueueWorker.hpp    # Job worker implementation
│   │   ├── QueueableJob.hpp   # Base job class
│   │   └── JobsHandler.hpp    # Job management
│   ├── Database/              # Database integration
│   ├── Storage/               # File storage utilities
│   ├── WebSocket/             # WebSocket support
│   ├── utils/                 # Utility classes
│   │   └── DocAPI.hpp         # OpenAPI documentation
│   └── boot.hpp               # Application bootstrap
├── tests/                     # Unit tests
└── example_metrics_usage.cpp  # Usage examples
```

### Design Patterns
- **MVC Pattern**: Controllers handle HTTP requests
- **Observer Pattern**: Metrics collection via callbacks
- **Template Metaprogramming**: Job serialization system
- **RAII**: Resource management throughout
- **Factory Pattern**: Job creation and handling

## 🧪 Testing

Run the test suite:
```bash
cd build
ctest -j 20 -C Debug -T test --output-on-failure
```

Individual test categories:
```bash
# Run specific tests
./bin/test_inputvalidators
./bin/test_worker_metrics
```

## 📚 Documentation

- **[Metrics System](METRICS.md)**: Comprehensive guide to job metrics collection
- **[Implementation Summary](IMPLEMENTATION_SUMMARY.md)**: Recent feature implementations
- **API Documentation**: Generate with `DOCAPI_ENABLED` flag

## 🛠️ Development

### Code Generation
Generate new job classes:
```bash
./newjob.sh MyNewJob "std::string"
```

### Code Style
The project uses enhanced security-focused static analysis tools:
```bash
# Format code
clang-format -i src/**/*.cpp src/**/*.hpp

# Run enhanced security-focused static analysis
clang-tidy src/**/*.cpp --checks='-*,cert-*,bugprone-*,clang-analyzer-security*,cppcoreguidelines-*' -- -std=c++20

# Run additional security analysis
cppcheck --enable=all --std=c++20 src/
```

## 🛡️ Security

This project implements enterprise-grade security scanning and follows security best practices:

### Security Scanning Tools
- **[CodeQL](https://codeql.github.com/)**: GitHub's semantic code analysis engine
- **Enhanced clang-tidy**: Security-focused static analysis with cert-*, bugprone-*, and security rules
- **cppcheck**: Additional static analysis for memory safety and undefined behavior
- **Dependabot**: Automated dependency vulnerability scanning
- **SBOM Generation**: Software Bill of Materials for supply chain security

### Security Features
- **Input Validation**: Comprehensive validation framework preventing injection attacks
- **SQL Injection Prevention**: Parameterized queries and input sanitization in database layer
- **Memory Safety**: RAII patterns, smart pointers, and bounds checking
- **Hardened Compilation**: Stack protection, position-independent executables, and security flags
- **Authentication Support**: JWT token validation and session management

### Security Policy
For security vulnerabilities, please refer to our [Security Policy](SECURITY.md). **DO NOT** create public issues for security vulnerabilities.

### Security Build Features
The build system includes security-hardened compilation:
```bash
# Security-hardened build
cmake .. -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_FLAGS="-Wall -Wextra -fstack-protector-strong -D_FORTIFY_SOURCE=3 -fPIE" \
  -DCMAKE_EXE_LINKER_FLAGS="-pie -Wl,-z,relro,-z,now"
```

### Building with Different Compilers
```bash
# GCC
export CC=gcc CXX=g++

# Clang (recommended)
export CC=clang-18 CXX=clang++-18

cmake .. -G Ninja
```

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch: `git checkout -b feature/amazing-feature`
3. Make your changes following the existing code style
4. Add tests for new functionality
5. Ensure all tests pass: `ctest`
6. Commit your changes: `git commit -m 'Add amazing feature'`
7. Push to the branch: `git push origin feature/amazing-feature`
8. Open a Pull Request

### Code Style Guidelines
- Use C++20 features when appropriate
- Follow RAII principles
- Add comprehensive tests for new features
- Document public APIs with Doxygen comments
- Use `clang-format` for consistent formatting

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- [Pistache HTTP Library](https://github.com/pistacheio/pistache)
- [Poco C++ Libraries](https://pocoproject.org/)
- [Google Test Framework](https://github.com/google/googletest)

## 📞 Support

- **Issues**: [GitHub Issues](https://github.com/Fabio3rs/cppapiframework/issues)
- **Discussions**: [GitHub Discussions](https://github.com/Fabio3rs/cppapiframework/discussions)

---

**Note**: This framework is actively developed and some features may be experimental. Please check the issues and documentation for the latest information.
