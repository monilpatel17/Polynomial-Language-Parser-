#!/bin/bash
#
# Test runner script for Polynomial Language Parser
# Usage: ./run_tests.sh
#

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Compile the program
echo -e "${YELLOW}Compiling...${NC}"
g++ -std=c++17 -Wall -o poly_parser inputbuf.cc lexer.cc parser.cc
if [ $? -ne 0 ]; then
    echo -e "${RED}Compilation failed!${NC}"
    exit 1
fi
echo -e "${GREEN}Compilation successful!${NC}"
echo ""

# Test counter
TESTS_PASSED=0
TESTS_FAILED=0
TESTS_TOTAL=0

# Function to run a single test
run_test() {
    local test_name="$1"
    local input_file="$2"
    local expected_output="$3"
    
    ((TESTS_TOTAL++))
    
    echo -n "Testing $test_name... "
    
    actual_output=$(./poly_parser < "$input_file" 2>&1)
    
    if [ "$actual_output" == "$expected_output" ]; then
        echo -e "${GREEN}PASSED${NC}"
        ((TESTS_PASSED++))
    else
        echo -e "${RED}FAILED${NC}"
        echo "  Expected: $expected_output"
        echo "  Actual:   $actual_output"
        ((TESTS_FAILED++))
    fi
}

# Function to run test expecting syntax error
run_syntax_error_test() {
    local test_name="$1"
    local input_file="$2"
    
    ((TESTS_TOTAL++))
    
    echo -n "Testing $test_name (syntax error)... "
    
    actual_output=$(./poly_parser < "$input_file" 2>&1)
    
    if [[ "$actual_output" == *"SYNTAX ERROR"* ]]; then
        echo -e "${GREEN}PASSED${NC}"
        ((TESTS_PASSED++))
    else
        echo -e "${RED}FAILED${NC}"
        echo "  Expected: SYNTAX ERROR"
        echo "  Actual:   $actual_output"
        ((TESTS_FAILED++))
    fi
}

echo "========================================"
echo "Running Polynomial Parser Tests"
echo "========================================"
echo ""

# Task 2 Tests - Program Execution
echo "--- Task 2: Program Execution ---"
run_test "Basic evaluation (f(5) where f=2x+3)" "tests/test_basic_task2.txt" "13"
run_test "Square and add (3^2 + 4)" "tests/test_task2_evaluation.txt" "13"
run_test "Nested evaluation g(f(3)) = (2*3+1)^2" "tests/test_nested_eval.txt" "49"
run_test "Numeric arguments add(3,5), mult(4,6)" "tests/test_numeric_args.txt" "8
24"
run_test "Constant polynomial (42)" "tests/test_constant_poly.txt" "42
constant: 0"
echo ""

# Semantic Error Tests
echo "--- Semantic Errors ---"
run_test "Error 1: Duplicate polynomial name" "tests/test_sem_err1_duplicate.txt" "Semantic Error Code 1: 5"
run_test "Error 2: Undefined var in poly body" "tests/test_sem_err2_invalid_name.txt" "Semantic Error Code 2: 3"
run_test "Error 3: Undeclared polynomial call" "tests/test_sem_err3_undeclared.txt" "Semantic Error Code 3: 6"
run_test "Error 4: Wrong argument count" "tests/test_sem_err4_wrong_args.txt" "Semantic Error Code 4: 6"
echo ""

# Syntax Error Tests
echo "--- Syntax Errors ---"
run_syntax_error_test "Missing semicolon" "tests/test_syntax_err_semicolon.txt"
run_syntax_error_test "Missing TASKS section" "tests/test_syntax_err_no_tasks.txt"
run_syntax_error_test "Missing parenthesis" "tests/test_syntax_err_paren.txt"
echo ""

# Task 3 Tests - Uninitialized Variables
echo "--- Task 3: Uninitialized Variable Warnings ---"
run_test "Uninit var in poly argument" "tests/test_task3_uninit_warn.txt" "Warning Code 1: 5"
run_test "Uninit var in OUTPUT" "tests/test_task3_output_uninit.txt" "Warning Code 1: 5"
run_test "Multiple uninit warnings" "tests/test_task3_multi_warn.txt" "Warning Code 1: 6 7"
echo ""

# Task 4 Tests - Useless Assignments
echo "--- Task 4: Useless Assignment Warnings ---"
run_test "Useless assignment (overwritten)" "tests/test_task4_useless_assign.txt" "Warning Code 2: 7"
run_test "Useless INPUT (never used)" "tests/test_task4_useless_input.txt" "Warning Code 2: 6"
run_test "Multiple useless assignments" "tests/test_task4_multi_useless.txt" "Warning Code 2: 6 7 8"
echo ""

# Task 5 Tests - Polynomial Degrees
echo "--- Task 5: Polynomial Degree Computation ---"
run_test "Various polynomial degrees" "tests/test_task5_degrees.txt" "f: 3
g: 5
h: 0
linear: 1"
run_test "High degree polynomials" "tests/test_high_degree.txt" "high: 10
product: 12"

echo ""
echo "========================================"
echo "Test Summary"
echo "========================================"
echo -e "Passed: ${GREEN}$TESTS_PASSED${NC}"
echo -e "Failed: ${RED}$TESTS_FAILED${NC}"
echo "Total:  $TESTS_TOTAL"
echo ""

if [ $TESTS_FAILED -eq 0 ]; then
    echo -e "${GREEN}All tests passed!${NC}"
    exit 0
else
    echo -e "${RED}Some tests failed.${NC}"
    exit 1
fi
