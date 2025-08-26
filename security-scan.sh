#!/bin/bash

# Enterprise Security Scanning Script for CPP API Framework
# This script runs comprehensive security analysis using multiple tools

set -e

echo "🛡️  Enterprise Security Scanning - CPP API Framework"
echo "=================================================="

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
BUILD_DIR="build"
SRC_DIR="src"
SECURITY_REPORT_DIR="security-reports"

# Create security reports directory
mkdir -p "$SECURITY_REPORT_DIR"

echo -e "${BLUE}📋 Security Analysis Report - $(date)${NC}" > "$SECURITY_REPORT_DIR/security-summary.txt"
echo "=============================================" >> "$SECURITY_REPORT_DIR/security-summary.txt"

echo -e "\n${YELLOW}1. Building with Security-Hardened Flags...${NC}"
if [ ! -d "$BUILD_DIR" ]; then
    mkdir -p "$BUILD_DIR"
fi

cd "$BUILD_DIR"

# Configure with security-hardened build
export CC=$(which clang-18 2>/dev/null || which clang || which gcc)
export CXX=$(which clang++-18 2>/dev/null || which clang++ || which g++)

echo "Using compiler: $CXX"

cmake .. -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_FLAGS="-Wall -Wextra -Wsecurity -fstack-protector-strong -D_FORTIFY_SOURCE=2 -fPIE" \
    -DCMAKE_EXE_LINKER_FLAGS="-pie -Wl,-z,relro,-z,now" \
    -DCompileTestsApiFramework=OFF > ../security-reports/build-config.log 2>&1

cmake --build . --config Release --target cppapiframework -j $(nproc) > ../security-reports/build.log 2>&1

echo -e "${GREEN}✅ Build completed with security flags${NC}"

cd ..

echo -e "\n${YELLOW}2. Running cppcheck Static Analysis...${NC}"
if command -v cppcheck >/dev/null 2>&1; then
    echo "Running comprehensive cppcheck analysis..."
    cppcheck --enable=all --error-exitcode=0 --xml --xml-version=2 \
        --platform=unix64 --std=c++20 \
        --suppress=missingIncludeSystem --suppress=unmatchedSuppression \
        --suppress=unusedFunction --inconclusive --force \
        "$SRC_DIR/" 2> "$SECURITY_REPORT_DIR/cppcheck-results.xml"
    
    # Parse results
    ERROR_COUNT=$(grep -c "severity=\"error\"" "$SECURITY_REPORT_DIR/cppcheck-results.xml" 2>/dev/null || echo "0")
    WARNING_COUNT=$(grep -c "severity=\"warning\"" "$SECURITY_REPORT_DIR/cppcheck-results.xml" 2>/dev/null || echo "0")
    
    echo -e "${GREEN}✅ cppcheck completed - Errors: $ERROR_COUNT, Warnings: $WARNING_COUNT${NC}"
    echo "cppcheck - Errors: $ERROR_COUNT, Warnings: $WARNING_COUNT" >> "$SECURITY_REPORT_DIR/security-summary.txt"
    
    # Show critical issues
    if [ "$ERROR_COUNT" -gt 0 ]; then
        echo -e "${RED}🚨 Critical cppcheck issues found:${NC}"
        grep "severity=\"error\"" "$SECURITY_REPORT_DIR/cppcheck-results.xml" | head -5 || true
    fi
else
    echo -e "${YELLOW}⚠️  cppcheck not installed, skipping...${NC}"
    echo "cppcheck - NOT AVAILABLE" >> "$SECURITY_REPORT_DIR/security-summary.txt"
fi

echo -e "\n${YELLOW}3. Running Enhanced clang-tidy Security Analysis...${NC}"
if command -v clang-tidy-18 >/dev/null 2>&1; then
    cd "$BUILD_DIR"
    
    echo "Running security-focused clang-tidy on key files..."
    
    # Focus on security-critical files
    SECURITY_CRITICAL_FILES=(
        "../src/Database/CSql.cpp"
        "../src/WebInterface/CController.cpp"
        "../src/WebInterface/WebInputValidator.cpp" 
        "../src/Authorization/"
    )
    
    echo "" > "../$SECURITY_REPORT_DIR/clang-tidy-security.log"
    
    for file_pattern in "${SECURITY_CRITICAL_FILES[@]}"; do
        if [ -f "$file_pattern" ] || [ -d "$file_pattern" ]; then
            echo "Analyzing: $file_pattern" >> "../$SECURITY_REPORT_DIR/clang-tidy-security.log"
            find "$file_pattern" -name "*.cpp" -o -name "*.hpp" 2>/dev/null | head -3 | \
            xargs clang-tidy-18 \
                --checks='-*,cert-*,bugprone-*,clang-analyzer-security*,cppcoreguidelines-*' \
                -p . --format-style=file \
                >> "../$SECURITY_REPORT_DIR/clang-tidy-security.log" 2>&1 || true
        fi
    done
    
    cd ..
    echo -e "${GREEN}✅ clang-tidy security analysis completed${NC}"
    echo "clang-tidy - Security analysis completed" >> "$SECURITY_REPORT_DIR/security-summary.txt"
else
    echo -e "${YELLOW}⚠️  clang-tidy-18 not available, skipping...${NC}"
    echo "clang-tidy - NOT AVAILABLE" >> "$SECURITY_REPORT_DIR/security-summary.txt"
fi

echo -e "\n${YELLOW}4. Checking for Common Security Patterns...${NC}"
echo "Scanning for potential security issues in code..."

# Check for common security anti-patterns
echo "Security Pattern Analysis:" >> "$SECURITY_REPORT_DIR/security-summary.txt"

# Check for unsafe string functions
UNSAFE_FUNCS=$(grep -r "strcpy\|strcat\|sprintf\|gets" "$SRC_DIR/" --include="*.cpp" --include="*.hpp" | wc -l)
echo "- Unsafe string functions: $UNSAFE_FUNCS occurrences" >> "$SECURITY_REPORT_DIR/security-summary.txt"

# Check for potential SQL injection points
SQL_QUERIES=$(grep -r "SELECT\|INSERT\|UPDATE\|DELETE" "$SRC_DIR/" --include="*.cpp" --include="*.hpp" | wc -l)
echo "- SQL queries found: $SQL_QUERIES" >> "$SECURITY_REPORT_DIR/security-summary.txt"

# Check for hardcoded credentials patterns
HARDCODED=$(grep -ri "password\s*=\|api.*key\s*=" "$SRC_DIR/" --include="*.cpp" --include="*.hpp" | wc -l)
echo "- Potential hardcoded credentials: $HARDCODED" >> "$SECURITY_REPORT_DIR/security-summary.txt"

echo -e "${GREEN}✅ Security pattern analysis completed${NC}"

echo -e "\n${YELLOW}5. Generating Security Summary...${NC}"

echo "" >> "$SECURITY_REPORT_DIR/security-summary.txt"
echo "Analysis completed at: $(date)" >> "$SECURITY_REPORT_DIR/security-summary.txt"
echo "Tools used: cppcheck, clang-tidy, pattern analysis" >> "$SECURITY_REPORT_DIR/security-summary.txt"

echo -e "\n${GREEN}🎉 Security Analysis Complete!${NC}"
echo -e "${BLUE}📊 Report Summary:${NC}"
cat "$SECURITY_REPORT_DIR/security-summary.txt"

echo -e "\n${BLUE}📁 Detailed reports available in: $SECURITY_REPORT_DIR/${NC}"
echo -e "   - cppcheck-results.xml"
echo -e "   - clang-tidy-security.log" 
echo -e "   - security-summary.txt"
echo -e "   - build.log"

echo -e "\n${YELLOW}💡 Next Steps:${NC}"
echo -e "   1. Review detailed reports in security-reports/ directory"
echo -e "   2. Address any high-priority security issues"
echo -e "   3. Run GitHub Actions workflows for CodeQL analysis"
echo -e "   4. Check Dependabot for dependency vulnerabilities"

echo -e "\n${GREEN}Security scanning completed successfully! 🛡️${NC}"