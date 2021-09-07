/* © 2014 Silicon Laboratories Inc.
 */

/*
 * mock_control.c
 *
 *  Created on: Aug 14, 2015
 *      Author: trasmussen
 */

#include "mock_control.h"
#include <string.h>
#include "unity.h"

#define MOCK_CALL_DB_SIZE  1024 /**< Size of the mock call database. */
#define FAKE_CALL_DB_SIZE  255  /**< Size of the fake call database. */
#define STUB_CALL_DB_SIZE  255  /**< Size of the stub call database. */
#define MAX_MESSAGE_LENGTH 255  /**< Size of error message length, increase value if long messages are needed. */

const mock_t default_entry = {
  .p_func_name      = "default",
  .expect_arg       = {{0}, {0}, {0}, {0}},
  .actual_arg       = {{0}, {0}, {0}, {0}},
  .compare_rule_arg = {COMPARE_STRICT, COMPARE_STRICT, COMPARE_STRICT, COMPARE_STRICT},
  .error_code       = {0},
  .return_code      = {0},
  .executed         = false
};

static mock_t   mock_call_db[MOCK_CALL_DB_SIZE];
static uint32_t mock_db_idx;

static stub_t   stub_call_db[STUB_CALL_DB_SIZE];
static uint32_t stub_db_idx;

static fake_t   fake_call_db[FAKE_CALL_DB_SIZE];
static uint32_t fake_db_idx;

uint32_t g_mock_index;

void mock_call_expect_ex(uint32_t line_number, const char * p_func_name, mock_t ** pp_mock)
{
  // Reset the entry before usage.
  mock_call_db[mock_db_idx] = default_entry;

  // Assign entry and increment.
  *pp_mock                = &mock_call_db[mock_db_idx];
  (*pp_mock)->p_func_name = p_func_name;
  (*pp_mock)->executed    = false;
  (*pp_mock)->line_number = line_number;
  mock_db_idx++;
}

void mock_calls_clear(void)
{
  mock_db_idx = 0;
  stub_db_idx = 0;
  fake_db_idx = 0;
}

void mock_calls_verify(void)
{
  bool     failed = false;
  uint16_t i;
  for (i = 0; i < mock_db_idx; i++)
  {
    if (false == mock_call_db[i].executed)
    {
      char error_msg[MAX_MESSAGE_LENGTH] = "- Expected mock call never occurred: ";
      sprintf(error_msg, "- Expected mock call never occurred: %s(...) at %s:%lu - ", mock_call_db[i].p_func_name, Unity.CurrentTestName, (long unsigned)mock_call_db[i].line_number);
      UnityPrint(error_msg);

      failed = true;
    }
    mock_call_db[i] = default_entry;
  }
  mock_db_idx = 0;
  stub_db_idx = 0;
  fake_db_idx = 0;

  if (failed)
  {
    TEST_FAIL_MESSAGE("Expected mock calls never occurred, see list for details.");
  }
}

bool mock_call_find(char const * const p_function_name, mock_t ** pp_mock)
{
  uint32_t i;
  for (i = 0; i < mock_db_idx; i++)
  {
    if ((false == mock_call_db[i].executed) &&
        strcmp(mock_call_db[i].p_func_name, p_function_name) == 0)
    {
      *pp_mock = &mock_call_db[i];
      g_mock_index = i;
      (*pp_mock)->executed = true;
      return true;
    }
  }
  mock_db_idx = 0;
  stub_db_idx = 0;
  fake_db_idx = 0;

  static char error_msg[MAX_MESSAGE_LENGTH] = "Unexpected mock call occurred: ";
  strcat(error_msg, p_function_name);
  UNITY_TEST_FAIL(0, error_msg);
  return false;
}

bool mock_call_used_as_stub(char const * const p_file_name, char const * const p_function_name)
{
  uint32_t i;
  for (i = 0; i < stub_db_idx; i++)
  {
    if ((strcmp(stub_call_db[i].p_name, p_function_name) == 0) ||
        (strcmp(stub_call_db[i].p_name, p_file_name) == 0))
    {
      return true;
    }
  }
  return false;
}

void mock_call_use_as_stub(const char * const p_name)
{
  // Assign entry and increment.
  stub_call_db[stub_db_idx++].p_name = p_name;
}

bool mock_call_used_as_fake(char const * const p_file_name, char const * const p_function_name)
{
  uint32_t i;
  for (i = 0; i < fake_db_idx; i++)
  {
    if ((strcmp(fake_call_db[i].p_name, p_function_name) == 0) ||
        (strcmp(fake_call_db[i].p_name, p_file_name) == 0))
    {
      return true;
    }
  }
  return false;
}

void mock_call_use_as_fake(char * const p_name)
{
  // Assign entry and increment.
  fake_call_db[fake_db_idx++].p_name = p_name;
}

