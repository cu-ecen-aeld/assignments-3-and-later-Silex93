cmake_minimum_required(VERSION 3.0.0)
project(aesd-assignments)
# A list of all automated test source files
# At minimum it should include the files in the test/assignmentX directory
# Students can also add paths to their own test files using a ../ prefix to
# reference this working directory

set(CMAKE_C_FLAGS "-pthread")

set(AUTOTEST_SOURCES
    test/assignment1/Test_hello.c
    test/assignment1/Test_assignment_validate.c
)
# A list of all files containing test code that is used for assignment validation
set(TESTED_SOURCE
    ../examples/autotest-validate/autotest-validate.c
)
add_subdirectory(assignment-autotest)

# Add an executable for assignment 2 tests
set(AESDSOCKET_SOURCES
        server/aesdsocket.c
        server/queue.h
)

add_executable(aesdsocket ${AESDSOCKET_SOURCES})

# Link any necessary libraries to the assignment2 executable
# target_link_libraries(assignment2 some_library)

# Optionally, you can install the assignment2 executable
# install(TARGETS assignment2 DESTINATION bin)