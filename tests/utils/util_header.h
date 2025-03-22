#define TEST(expected, actual) test_code(expected, actual, #actual)
#define TEST_PROGRAM(expected, code_str) run_test(expected, code_str)
#define TEST_ERROR(code_str) run_error_test(code_str)