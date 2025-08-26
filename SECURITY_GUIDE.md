# Security Scanning Guide

This guide demonstrates how to use the comprehensive enterprise-grade security scanning tools implemented in the CPP API Framework.

## 🛡️ Overview

The framework includes multiple layers of security analysis:

- **Static Application Security Testing (SAST)**: CodeQL, clang-tidy, cppcheck
- **Software Composition Analysis (SCA)**: Dependabot, SBOM generation  
- **Security-Hardened Build**: Compiler flags, linker options
- **Manual Security Scanning**: Interactive tools and scripts

## 🚀 Quick Start

### Run Complete Security Analysis

```bash
# Run all security tools at once
./security-scan.sh
```

This script will:
1. Build with security-hardened flags
2. Run cppcheck static analysis
3. Execute enhanced clang-tidy security checks
4. Perform security pattern analysis  
5. Generate comprehensive reports

### Individual Security Tools

#### 1. Security-Hardened Build

```bash
mkdir build && cd build

# Configure with security flags
export CC=$(which clang-18)
export CXX=$(which clang++-18)

cmake .. -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_FLAGS="-Wall -Wextra -Wsecurity -fstack-protector-strong -D_FORTIFY_SOURCE=2 -fPIE" \
  -DCMAKE_EXE_LINKER_FLAGS="-pie -Wl,-z,relro,-z,now"

# Build
cmake --build . --config Release --target cppapiframework -j $(nproc)
```

#### 2. Static Analysis with cppcheck

```bash
# Comprehensive analysis
cppcheck --enable=all --std=c++20 --platform=unix64 \
  --suppress=missingIncludeSystem --suppress=unmatchedSuppression \
  --inconclusive --force src/

# Focus on security issues only
cppcheck --enable=warning,style,performance,portability,information \
  --std=c++20 src/ --xml --xml-version=2 2> security-issues.xml
```

#### 3. Enhanced Security clang-tidy

```bash
cd build

# Security-focused analysis
find ../src -name "*.cpp" | xargs clang-tidy-18 \
  --config-file=../.clang-tidy \
  --checks='-*,cert-*,bugprone-*,clang-analyzer-security*,cppcoreguidelines-*' \
  -p .

# Focus on critical security files
clang-tidy-18 ../src/Database/CSql.cpp ../src/WebInterface/CController.cpp \
  --checks='-*,cert-*,bugprone-*,clang-analyzer-security*' \
  -p . --format-style=file
```

## 🤖 Automated Security Scanning

### GitHub Actions Workflows

#### CodeQL Analysis
- **File**: `.github/workflows/codeql.yml`
- **Triggers**: Push to main/develop, PRs, weekly schedule
- **Features**: Semantic analysis with security-extended queries

#### Security Analysis & SBOM
- **File**: `.github/workflows/security-analysis.yml`  
- **Features**: cppcheck, clang-tidy, SBOM generation, vulnerability scanning
- **Reports**: Uploaded as artifacts

#### Dependabot
- **File**: `.github/dependabot.yml`
- **Features**: Weekly dependency vulnerability scanning
- **Coverage**: GitHub Actions, CMake, Docker dependencies

### Viewing Results

```bash
# After running security-scan.sh, check reports:
ls security-reports/

# View summary
cat security-reports/security-summary.txt

# Check cppcheck results
grep "error\|warning" security-reports/cppcheck-results.xml

# Review clang-tidy findings
cat security-reports/clang-tidy-security.log
```

## 📊 Understanding Security Reports

### cppcheck Results

```xml
<error id="arrayIndexOutOfBounds" severity="error" 
       msg="Array index out of bounds" file="src/example.cpp" line="42"/>
```

**Priority**: High - Fix immediately  
**Action**: Review array access patterns

### clang-tidy Security Warnings

```
warning: function 'strcpy' is not bounds-checking [cert-msc24-c]
warning: potential buffer overflow [bugprone-buffer-overflow]
```

**Priority**: High - Replace with safe alternatives  
**Action**: Use `strncpy` or `std::string`

### Dependabot Alerts

GitHub will automatically create PRs for:
- Dependency vulnerabilities
- Security updates
- Version updates with security fixes

## 🔍 Manual Security Review

### Security-Critical Files

Focus manual review on:

```bash
# Database layer - SQL injection risks
src/Database/CSql.hpp
src/Database/CSql.cpp

# Input validation - injection vulnerabilities  
src/WebInterface/WebInputValidator.cpp
src/utils/InputValidators.cpp

# Authentication - access control
src/Authorization/

# Network handling - protocol vulnerabilities
src/WebInterface/CController.cpp
src/WebSocket/
```

### Security Checklist

- [ ] All user inputs are validated
- [ ] SQL queries use parameterized statements
- [ ] No hardcoded credentials in source
- [ ] Memory allocations are bounds-checked
- [ ] Error messages don't leak sensitive information
- [ ] Authentication mechanisms are secure
- [ ] Session management is implemented correctly

## 🛠️ Customizing Security Analysis

### Enhanced clang-tidy Configuration

Edit `.clang-tidy` to add more checks:

```yaml
Checks: 'clang-diagnostic-*,clang-analyzer-*,cert-*,bugprone-*,
         cppcoreguidelines-*,hicpp-*,modernize-*,performance-*,
         readability-*,concurrency-*,misc-*'
```

### Custom cppcheck Rules

```bash
# Create custom suppression file
echo "missingIncludeSystem" > cppcheck-suppressions.txt
echo "unusedFunction:tests/*" >> cppcheck-suppressions.txt

# Use custom suppressions
cppcheck --suppressions-list=cppcheck-suppressions.txt src/
```

### Security Pattern Detection

```bash
# Check for specific security anti-patterns
grep -r "strcpy\|strcat\|sprintf\|gets" src/ --include="*.cpp" --include="*.hpp"

# Find potential SQL injection points
grep -r "\"SELECT\|\"INSERT\|\"UPDATE\|\"DELETE" src/ --include="*.cpp" --include="*.hpp"

# Look for hardcoded secrets
grep -ri "password\s*=\|api.*key\s*=\|token\s*=" src/ --include="*.cpp" --include="*.hpp"
```

## 📈 Continuous Security Monitoring

### Integration with CI/CD

```yaml
# Example GitHub Actions step
- name: Security Analysis
  run: |
    ./security-scan.sh
    
    # Fail build on critical issues
    if [ $(grep -c "severity=\"error\"" security-reports/cppcheck-results.xml) -gt 0 ]; then
      echo "Critical security issues found"
      exit 1
    fi
```

### Metrics and Monitoring

Track security improvements:
- Number of security warnings over time
- Dependency vulnerability count
- Code coverage of security tests
- Time to fix security issues

## 🔧 Troubleshooting

### Common Issues

**cppcheck false positives**: 
```bash
# Suppress specific warnings
cppcheck --suppress=unusedFunction src/
```

**clang-tidy build errors**:
```bash
# Ensure compilation database exists
cd build && cmake .. -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
```

**Missing security tools**:
```bash
# Install required tools
sudo apt install clang-18 clang-tidy-18 cppcheck
```

## 📚 Additional Resources

- [OWASP Secure Coding Practices](https://owasp.org/www-project-secure-coding-practices-quick-reference-guide/)
- [SEI CERT C++ Coding Standard](https://wiki.sei.cmu.edu/confluence/pages/viewpage.action?pageId=88046682)
- [clang-tidy Security Checks Documentation](https://clang.llvm.org/extra/clang-tidy/)
- [cppcheck Manual](http://cppcheck.sourceforge.net/manual.pdf)

---

**Security is a continuous process. Regularly run these tools and keep dependencies updated!** 🛡️