#include "unity.h"
#include <stdbool.h>
#include <stdlib.h>
#include "../../examples/autotest-validate/autotest-validate.h"
#include "../../assignment-autotest/test/assignment1/username-from-conf-file.h"
// File Modified to include test validate username functionality

void test_validate_my_username()
{
    //Get the harcoded username
    const char * hardcoded_username = my_username();
    //Get the github username
    char * config_username = malloc_username_from_conf_file();
    //Run the test
    TEST_ASSERT_EQUAL_STRING_MESSAGE(hardcoded_username,config_username,"Username Validation Failed");
    //Free the memory used by the malloc function
    free(config_username);
}
