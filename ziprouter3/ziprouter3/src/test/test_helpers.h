/* © 2017 Silicon Laboratories Inc.
 */

#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H

#include <stdint.h>
#include <stdio.h>
#include <lib/zgw_log.h>

#define TEST_CREATE_LOG_NAME __FILE__".log"

/** \defgroup test_helpers Simple Test Framework
 *
 * The simple test framework provides a small set of unit test helpers to handle
 *
 * - Case definition and counting
 * - Condition checking
 * - Failure counting
 * - Final summary
 * - Logging to file or stdout with controllable verbosity
 *
 * The framework fits with ctest as the primary test runner.
 *
 * @{
 */

/** Total number of errors (aka failed checks) in this test case.
 */
extern int numErrs;

/** The verbosity of test logging.
 *
 * As long as the test case does not have an opinion, log
 * everything. */
extern int verbosity;

/** Verbosity levels */
enum {
    test_always = 0,
    test_suite_start_stop = 1,
    test_case_start_stop = 2,
    test_comment = 3,
    test_verbose = 4,
    test_number_of_levels
} test_verbosity_levels;

/** Validator function.  Check if argument is zero and increment numErrs if not.
 *
 * Print msg and fail statement to stdout on failure.
 * Print message and passed statement to log at level 2 on success.
*/
void check_zero(int ii, char *msg);

/** Validator function.  Check if argument is zero and increment numErrs if not.
 *
 * Print msg and fail statement to stdout on failure.
 * Print message and passed statement to log at level 2 on success.
*/
void check_not_null(/*@null@*/void *pvs, char *msg);

/** Validator function.  Check if argument is zero and increment numErrs if not.
 *
 * Print msg and fail statement to stdout on failure.
 * Print message and passed statement to log at level 2 on success.
*/
void check_null(/*@null@*/void *pvs, char *msg);

/** Validator function.  Check if argument is zero and increment numErrs if not.
 *
 * Print msg and fail statement to stdout on failure.
 * Print message and passed statement to log at level 2 on success.
*/
void check_true(int8_t ii, char *msg);

/**
 * Compare two byte arrays and print the differences (if any).
 *
 */
void check_mem(const uint8_t *expected, const uint8_t *was, uint16_t len, const char *errmsg, const char *msg);

FILE *test_create_log(const char *logname);

/** Print out final result of the test run.
 *
 * Should typically be called at the end of main(), before returning
 * numErrs. */
void close_run();


/** Print out start text for a related group of validations on that could be considered one case.
 *
 * Sometimes several validations are needed to confirm a certain
 * behaviour (eg, first check that a pointer is not null, then check
 * that it points to the right thing).  This function allows you to
 * group the subsequent validations under a common name and print log
 * statements reflecting this hierarchy.
 */
void start_case(const char *str, /*@null@*/FILE *logfile);

/** Print out a summary of the results of the validations since this case was started.

 This function does not perform any validation or affect the result in
 any way.  It is only useful if you want more structured logging. */
void close_case(const char *str);


#ifdef ZGW_LOG
zgw_log_id_declare(TC);
#define test_print(lvl, fmt, ...) do {zgw_log_to(TC, lvl, fmt, ##__VA_ARGS__); } while (0)
#else
/** Print to stdout if verbosity is at least lvl */
#define test_print(lvl, fmt, ...) __test_print(lvl, fmt, ##__VA_ARGS__)

/** Helper function for non-log printing */
void __test_print(int lvl, const char *fmt, ...);
#endif

/** Print with some fancy formatting */
void test_print_suite_title(int lvl, const char *fmt, ...);

 /* @}
 */
#endif
