#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/wait.h>
//#include <csse2310a3.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <time.h>


// Enum for storing command line errors
typedef enum {
    COMMAND_LINE_ERROR_EXIT = 9
} CommandLineErrors;

// Enum for job file errors
typedef enum {
    JOB_SPEC_FILE_ERROR = 8,  // Error for opening job specification file.
    SYNTAX_ERROR = 1,
    TEST_ID_ERROR = 17,  // Exit status for multiple tests with the same ID.
    INPUT_FILE_ERROR = 13,  // Exit status for opening input file error.
    EMTPY_JOB_FILE = 5
} JobFileErrors;

// Enum for expected outputs
typedef enum {
    DIRECTORY_MAKE_ERROR = 3,
    OPEN_FILE_ERROR = 11
} ExpectedOutputs;

// Enum for running test jobs
typedef enum {
    READ_END = 0,
    WRITE_END = 1,
    PROCESS_A_EXIT = 97,
    PROCESS_B_EXIT = 98,
    PROCESS_C_EXIT = 99,
    PROCESS_A_INDEX = 0,
    PROCESS_B_INDEX = 1,
    PROCESS_C_INDEX = 2,
    OUTPUT_MATCHED = 0,
    OUTPUT_DIFFERENT = 1
} RunningTestJobs;

// Enum for reporting overall test result
typedef enum {
    OVERALL_TEST_RESULT_PASS = 0,
    OVERALL_TEST_RESULT_FAIL = 7
} ReportOverallResult;

// Struct for command line args
typedef struct {
    char *dir;
    bool regen;
    char *jobFile;  // Name of the job file containing tests
    char *program;  // Name of the program to execute
} CommandLineArgs;

// Struct to hold individual test information
typedef struct {
    char *testID;  // Holds the test ID (can be a string).
    char *testInputFileName;  // Input file name for the test.
    FILE *testInputFileHandler;  // Input file handler.
    char **testArgs;  // Arguments for the test.
    int testArgsCount;  // Number of arguments for the test.
    
    // For pipes:
    int standardOutCmp[2];  // Handles standard output comparison.
    int standardErrorCmp[2];  // Handles standard error comparison.
    
    // File descriptors for test and expected files:
    FILE *testStandOutFileHandler;
    FILE *testStandErrorFileHandler;
    FILE *testExitStatusFileHandler;
    FILE *goodStandOutFileHandler;
    FILE *goodStandErrorFileHandler;
    FILE *goodExitStatusFileHandler;
    int goodExitStatus;  // Stores the good exit status.
} IndividualTest;

// Struct to hold test file list
typedef struct {
    int numTests;  // Number of tests in the job file.
    IndividualTest *tests;  // Reference to IndividualTest structs.
    int testsCompleted;
    int testsPassed;  // Number of passed tests.
    int testsFailed;  // Number of failed tests.
} TestFileList;

/* command_line_error():
----------------------
Handles command line errors by printing the correct usage and exiting the program.
This function causes the program to exit with status 9 when called.
Returns: None.
*/
void command_line_error(void) {
    fprintf(stderr,
            "Usage: testuqwordladder [--showdiff N] [--dir dir] "
            "[--regen] jobspecfile program\n");
    exit(COMMAND_LINE_ERROR_EXIT);
}

/* command_line_arguments
Handles and validates command-line arguments and populates the commandLineArgs
struct with detauls on the directory, whether to regenerate expected output
files, the job file, and the program to execute.

arg1: argc - The number of command-line arguments.
arg2: argv[] - An array of command-line arguments.
arg3: parameters - A pointer to the CommandLineArgs structure to populate.

Returns: None (This function modifies the CommandLineArgs structure passed
         as an argument)
Errors: Iv invalid arguments are present, this function calls
        the  command_line_error() and exits the program.
*/
void command_line_arguments(int argc, char *argv[], 
CommandLineArgs *parameters) {
    // Validate the number of arguments
    if (argc < 3 || argc > 6) {
        command_line_error();
    }
    
    // Initialize command line struct fields
    parameters->dir = NULL;
    parameters->regen = false;
    parameters->jobFile = NULL;
    parameters->program = NULL;
    
    bool dirGiven = false;
    bool regenGiven = false;
    
    //Make the and second last arguments the program and jobFile respectively.
    parameters->program = argv[argc - 1];
    parameters->jobFile = argv[argc - 2];
    
    // Validate jobfile and program arguments
    if (argv[argc - 1][0] == '-' || argv[argc - 2][0] == '-') {
        command_line_error();
    }
    
    // Check for optional --dir and --regen flags (if argc is larger than 3)
    if (argc > 3) {
        for (int i = 1; i < argc - 2; i++) {
            if (strcmp(argv[i], "--dir") == 0) {
                if (dirGiven || i == argc - 3) {
                    command_line_error();
                }
                parameters->dir = argv[i + 1];
                dirGiven = true;
                i++;
            } else if (strcmp(argv[i], "--regen") == 0) {
                if (regenGiven) {
                    command_line_error();
                }
                parameters->regen = true;
                regenGiven = true;
            } else {
                command_line_error();
            }
        }
    }
    
    // Set the default directory if the directory is not given by the user. 
    if (!dirGiven) {
        parameters->dir = "./tmp";
    }
}



/*
open_job_file_error():
----------------------
This function is used in the case where the job file on the command line
cannot be opened.
This function will take in the job file path and then use it to print
the message to stderr and then exit with exit status 8.

arg1: jobFile (the job file path which was not able to be opened.)

Returns: None (This function exits the program)
*/
void open_job_file_error(char *jobFile) {
    fprintf(stderr, "testuqwordladder: Unable to open job file \"%s\"\n", jobFile);
    exit(JOB_SPEC_FILE_ERROR);
}

/*
comment_or_empty_line_check():
-----------------------------
This function checks if the specified line is a comment line (where
the first character is '#') or whether it is an empty line (where the first 
character on the line is '\n' or the string length is 0).
If the line is a comment or empty line, then the function will return true, 
else it will return false.

arg1: line - The string line to be checked.

Returns: True if the line is a comment or empty, false otherwise.
*/
bool comment_empty_line_check(char *line) {
    if (line[0] == '#' || line[0] == '\n' || strlen(line) == 0) {
        return true;
    } else {
        return false;
    }
}

/*
syntax_error():
--------------
This function will print the syntax error message to stderr in the 
format expected by the assignment specification file.

arg1: jobFile - The path of the job file where the syntax error occurred.
arg2: lineNum - The line number in the job file where the syntax error occurred.

Returns: None (This function exits the program with status 1).
*/
void syntax_error(char *jobFile, int lineNum) {
    fprintf(stderr, 
            "testuqwordladder: Syntax error on line %d of job file \"%s\"\n", 
            lineNum, jobFile);
    exit(SYNTAX_ERROR);
}

/*
check_syntax_error():
---------------------
Checks the syntax of each non-comment/empty line in the job file.
Exits with stderr message if an error is found (and an exit status of 1).

arg1: jobFile - The path of the job file being checked.
arg2: lineNum - The current line number in the job file.
arg3: line - The string line to be checked for syntax errors.

Returns: char **fields array, which is the result of splitting char *line, if syntax is correct.
*/
char **check_syntax_error(char *jobFile, int lineNum, char *line) {
    char **fields = split_string(line, '\t');
    
    int fieldCount = 0;
    while (fields[fieldCount] != NULL) {
        fieldCount++;
    }
    
    //If the number of feilds is less than 2 (the two mandotory fields
    //are not supplied), then run the syntax_error() function. 
    if (fieldCount < 2) {
        syntax_error(jobFile, lineNum);
    }
    
    //If the testID is empty, indicate a syntax error.
    if (strlen(fields[0]) == 0) {
        syntax_error(jobFile, lineNum);
    }
    
    //If the testID contains a '/' character, indicate a syntax error.
    if (strchr(fields[0], '/') != NULL) {
        syntax_error(jobFile, lineNum);
    }
    
    //If the input file name is empty, indicate a syntax error.
    if (strlen(fields[1]) == 0) {
        syntax_error(jobFile, lineNum);
    }
    
    //If all the test have passed, return the fieds array. 
    return fields;
}

/*
same_test_id_error():
---------------------
Used to call the error message for duplicate test IDs and exits with status 17.

arg1: jobFile - The path of the job file being checked.
arg2: lineNum - The current line number in the job file.

Returns: None (This function exits the program)
*/
void same_test_id_error(char *jobFile, int lineNum) {
    fprintf(stderr, 
            "testuqwordladder: Line %d of file \"%s\": "
            "duplicate test ID\n", lineNum, jobFile);
    exit(TEST_ID_ERROR);
}

/*
check_new_test_id():
--------------------
This function will take in the testID array, and the number of test IDs
in the array (int testIdCount). It will then iterate through this array and 
check whether a given testID is already in the array.

arg1: testIdArray - The array of existing test IDs.
arg2: testIdCount - The number of test IDs in the array.
arg3: testId - The new test ID to check.

Returns: True if the test ID is new, false otherwise.
*/

bool check_new_test_id(char **testIggdArray, int testIdCount, char *testId) {
    if (testIdCount > 0) {
        for (int i = 0; i < testIdCount; i++) {
            if (strcmp(testIdArray[i], testId) == 0) {
                return false;
            }
        }
    }
    return true;
}

/*
input_file_open_check():
------------------------
Checks if  a given file can be opened for reading. 

arg1: jobFile - The path of the job file being checked.
arg2: inputFile - The path of the input file to check.
arg3: lineNum - The current line number in the job file.

Returns: None
Errors: If the file cannot be opened, this function will print an error
        message to stderr and exit with status 13.

*/
void input_file_open_check(char *jobFile, char *inputFile, int lineNum) {
    FILE *inputFileHandler;
    if ((inputFileHandler = fopen(inputFile, "r")) == NULL) {
        fprintf(stderr, 
                "testuqwordladder: Unable to open file \"%s\" specified "
                "on line %d of file \"%s\"\n", inputFile, lineNum, jobFile);
        exit(INPUT_FILE_ERROR);
    }
    fclose(inputFileHandler);
}

/*
jobspec_file_empty():
---------------------
Prints an error message and exits if the jobspec file is empty.

arg1: jobFile - The path of the empty job file.

Returns: None (This function exits the program with status 5).
*/
void jobspec_file_empty(char *jobFile) {
    fprintf(stderr, "testuqwordladder: Test file \"%s\" was empty\n", jobFile);
    exit(EMTPY_JOB_FILE);

}



/*
free_two_dimensional_array():
-----------------------------
This function is used to free the memory allocated to the char**array. 
*/
void free_two_dimensional_array(char **array) {
    int i = 0;
    while (array[i] != NULL) {
        free(array[i]);
        i++;
    }
    free(array);
}

/*
free_test_id_array():
---------------------
Frees the memory allocated for a given array of test IDs.

arg1: array - The array of test IDs to free.
arg2: testIdCount - The number of test IDs in the array.

Returns: None
*/
void free_test_id_array(char **array, int testIdCount) {
    for (int i = 0; i < testIdCount; i++) {
        free(array[i]);
    }
    free(array);
}


/*
create_new_individual_test():
-----------------------------
Creates a new IndividualTest struct from an array (char **fields).


arg1: fields - The array of fields to populate the IndividualTest struct.

Returns: An IndividualTest struct populated with the given fields.
*/
IndividualTest create_new_individual_test(char **fields) {
    // Create a copy of the individualTest struct. 
    IndividualTest test;

    // Initialize variables in the struct with placeholder values. 
    test.testID = NULL;
    test.testInputFileName = NULL;
    test.testInputFileHandler = NULL;
    test.testArgsCount = 0;
    test.testArgs = NULL;

    // Copy testID and testInputFileName into the struct.
    test.testID = strdup(fields[0]);
    test.testInputFileName = strdup(fields[1]);

    // Create a file handler for the input file. 
    test.testInputFileHandler = fopen(test.testInputFileName, "r");

    // Count total elements in fields array. 
    int fieldCount = 0;
    while (fields[fieldCount] != NULL) {
        fieldCount++;
    }

    // Number of args is total fields minus 2.
    test.testArgsCount = fieldCount - 2;

    // Allocate memory for the args.
    test.testArgs = malloc((test.testArgsCount + 1) * sizeof(char *));

    // Copy the args into testArgs array.
    for (int j = 2; j < fieldCount; j++) {
        test.testArgs[j - 2] = strdup(fields[j]);
    } 

    // Last element of testArgs array is NULL. 
    test.testArgs[test.testArgsCount] = NULL;

    return test;
}

/*
add_individual_test():
----------------------
Adds an IndividualTest struct to an arry of IndividualTests 
in the TestFileList struct.

arg1: test - The IndividualTest struct to add.
arg2: testList - The TestFileList struct to which the test should be added.

Returns: None
*/
void add_individual_test(IndividualTest test, TestFileList *testList) {
    // Allocate memory for the test array in TestFileList struct.
    testList->tests = realloc(testList->tests, 
                              (testList->numTests) * sizeof(IndividualTest));

    // Add IndividualTest to TestFileList struct.
    testList->tests[testList->numTests - 1] = test;
}

/*
job_specification_file():
-------------------------
Performs various checks on the job file and populates the TestFileList struct.
These tests include checking for syntax errors, duplicate test IDs, and 
whether files can be opened.

arg1: parameters - The CommandLineArgs struct containing command line arguments.
arg2: testList - The TestFileList struct to be populated.

Returns: None
Errors: Exits with respective status codes if any checks fail.
*/

void job_specification_file(CommandLineArgs *parameters, TestFileList *testList) {
    FILE *jobfile;
    if ((jobfile = fopen(parameters->jobFile, "r")) == NULL) {
        open_job_file_error(parameters->jobFile); }
    // Initialize testList varibales and variables to used in while loop.
    testList->numTests = 0; testList->tests = NULL;
    char *line, **fields = NULL, **testIdArray = NULL;
    int currentLineNumber = 0, testIdCount = 0;

    //Iterate through the jobspec file until EOF is reached. 
    while ((line = read_line(jobfile)) != NULL) {
        currentLineNumber++;
        if (comment_empty_line_check(line)) {
            free(line);
            continue; }
        //Check syntax and if the test ID is already in the array.
        fields = check_syntax_error(parameters->jobFile,
                                    currentLineNumber, line);

        if (!check_new_test_id(testIdArray, testIdCount, fields[0])) {
            same_test_id_error(parameters->jobFile, currentLineNumber); } 
            else { //Add testID to array if it is not already in the array.
            testIdArray = realloc(testIdArray,
                                  (testIdCount + 1) * sizeof(char *));
            testIdArray[testIdCount] = strdup(fields[0]);
            testIdCount++; }

        //Check if input file can be opened.
        input_file_open_check(parameters->jobFile, fields[1],
                              currentLineNumber);
        // All checks passed, increment test count
        testList->numTests++;
        IndividualTest test = create_new_individual_test(fields);
        add_individual_test(test, testList);
        free(line); }
    //Check if job file is empty. 
    if (testList->numTests == 0) {
        jobspec_file_empty(parameters->jobFile); }
    free_test_id_array(testIdArray, testIdCount);
    fclose(jobfile);
}


/*
make_test_directory():
----------------------
Creates a new directory if it doesn't already exist.

arg1: dir - The name of the directory to create.

Returns: None
Errors: Exits with status code 3 if directory can't be created.
*/

void make_test_directory(char *dir) {
    if (mkdir(dir, S_IRWXU) == -1) {
        if (errno != EEXIST) {
            fprintf(stderr,
                    "testuqwordladder: Unable to create directory \"%s\"\n",
                    dir);
            exit(DIRECTORY_MAKE_ERROR);
        }
    }
}

/*
create_expected_output_file():
------------------------------
Creates expected output files for each test based on the testID.
These expected output files will be empty, and will be populated later if 
required.

arg1: testList - The TestFileList struct containing the tests.
arg2: dir - The directory where the files should be created.
arg3: index - The index of the current test in the TestFileList struct.

Returns: None
Errors: Exits with exit stats of 11 if any file can't be created or
        opened for writing.
*/
void create_expected_output_file(TestFileList *testList, char *dir, int index) {
    // Initialize buffer size for the filenames:
    int buffer = 100;
    char stdoutFileName[100], stderrFileName[100], exitFileName[100];
    
    // Use snprintf to generate the filenames.
    snprintf(stdoutFileName, buffer, "%s/%s.stdout", dir, 
             testList->tests[index].testID);
    snprintf(stderrFileName, buffer, "%s/%s.stderr", dir, 
             testList->tests[index].testID);
    snprintf(exitFileName, buffer, "%s/%s.exitstatus", dir, 
             testList->tests[index].testID);
    
    // Use the filenames to create the output files.
    // Exit if file creation failed.
    int stdoutFD = open(stdoutFileName, O_WRONLY | O_CREAT | O_TRUNC, 
                        S_IRWXU | S_IRGRP);
    if (stdoutFD == -1) {
        fprintf(stderr, 
                "testuqwordladder: Can't open file \"%s\" for writing\n",
                stdoutFileName);
        exit(OPEN_FILE_ERROR);
    }

    int stderrFD = open(stderrFileName, O_WRONLY | O_CREAT | O_TRUNC, 
                        S_IRWXU | S_IRGRP);
    if (stderrFD == -1) {
        fprintf(stderr, 
                "testuqwordladder: Can't open file \"%s\" for writing\n",
                stderrFileName);
        exit(OPEN_FILE_ERROR);
    }

    int exitFD = open(exitFileName, O_WRONLY | O_CREAT | O_TRUNC, 
                      S_IRWXU | S_IRGRP);
    if (exitFD == -1) {
        fprintf(stderr, 
                "testuqwordladder: Can't open file \"%s\" for writing\n",
                exitFileName);
        exit(OPEN_FILE_ERROR);
    }

    // If the files can be opened for writing, then wrap the file descriptor
    testList->tests[index].goodStandOutFileHandler = fdopen(stdoutFD, "w");
    testList->tests[index].goodStandErrorFileHandler = fdopen(stderrFD, "w");
    testList->tests[index].goodExitStatusFileHandler = fdopen(exitFD, "w");
}

/*
fill_expected_output():
-----------------------
Generates expected outputs by calling fork() and executing the good-uqword
ladder program. This function will fill the files that have already been 
created by the create_expected_output_file() function.

arg1: testList - The TestFileList struct containing the tests.
arg2: index - The index of the current test in the TestFileList struct.

Returns: None
Errors: None
*/
void fill_expected_output(TestFileList *testList, int index){
    // Create a variable to hold the process ID when calling fork.
    #include <sys/types.h>

    pid_t pid;
    char *programName = "good-uqwordladder";
    //Remove any existing content in the files.
    ftruncate(fileno(testList->tests[index].goodStandOutFileHandler), 0);
    ftruncate(fileno(testList->tests[index].goodStandErrorFileHandler), 0);
    ftruncate(fileno(testList->tests[index].goodExitStatusFileHandler), 0);
    // Call fork and allocate fork status to pid.
    pid = fork();

    // The child process:
    if (pid == 0) {
        // Redirec the standard input, standard output, and standard error.
        dup2(fileno(testList->tests[index].testInputFileHandler), STDIN_FILENO);
        dup2(fileno(testList->tests[index].goodStandOutFileHandler),
             STDOUT_FILENO);
        dup2(fileno(testList->tests[index].goodStandErrorFileHandler),
             STDERR_FILENO);

        // Create an array to store the program name and the command line arguments.
        int arraySize = testList->tests[index].testArgsCount + 2;
        char *arguments[arraySize];
        arguments[0] = programName;
        // Copy the command line arguments into the arguments array.
        for (int j = 1; j < arraySize; j++)
        {
            arguments[j] = testList->tests[index].testArgs[j - 1];
        }
        // execvp the program with the arguments array:
        execvp(arguments[0], arguments);
    }
    
    // The parent process:
    else if (pid > 0) {
        int status;
        // Wait for the child process to finish, and get exit status.
        waitpid(pid, &status, 0);
        int exit_status = WEXITSTATUS(status);
        //Write exit status to a file, and store the value in the goodExitStatus variable.
        fprintf(testList->tests[index].goodExitStatusFileHandler, "%d\n",
                exit_status);
        testList->tests[index].goodExitStatus = exit_status; 
    }
}


/*
is_regen_needed():
------------------
Checks if regeneration of test files is needed for a particular test. 

arg1: regen - Flag indicating if regeneration is needed.
arg2: jobFilePath - The path to the job file.
arg3: testList - The TestFileList struct containing the tests.
arg4: index - The index of the current test in the TestFileList struct.

Returns: True if regeneration is needed, otherwise False.
Errors: None
*/
bool is_regen_needed(bool regen, char *jobFilePath, TestFileList *testList,
                     int index) {
    if (regen) {
        return true;
    }
    struct stat jobFileStat, stdoutFileStat, stderrFileStat, exitFileStat;
    stat(jobFilePath, &jobFileStat);
    int goodStandOut = fstat(
        fileno(testList->tests[index].goodStandOutFileHandler),
        &stdoutFileStat);
    int goodStandError = fstat(
        fileno(testList->tests[index].goodStandErrorFileHandler),
        &stderrFileStat);
    int goodExitStatus = fstat(
        fileno(testList->tests[index].goodExitStatusFileHandler),
        &exitFileStat);

    if (goodStandOut == -1 || goodStandError == -1 || goodExitStatus == -1) {
        return true;
    }

    if (compare_timespecs(jobFileStat.st_mtim, stdoutFileStat.st_mtim) > 0) {
        return true;
    }
    if (compare_timespecs(jobFileStat.st_mtim, stderrFileStat.st_mtim) > 0) {
        return true;
    }
    if (compare_timespecs(jobFileStat.st_mtim, exitFileStat.st_mtim) > 0) {
        return true;
    }
    return false;
}

/*
close_file_handlers():
----------------------
Closes the file handlers for each test in the TestFileList struct.

arg1: testList - The TestFileList struct containing the tests.

Returns: None
Errors: None
*/
void close_file_handlers(TestFileList *testList) {
    for (int i = 0; i < testList->numTests; i++) {
        fclose(testList->tests[i].testInputFileHandler);
        fclose(testList->tests[i].goodStandOutFileHandler);
        fclose(testList->tests[i].goodStandErrorFileHandler);
        fclose(testList->tests[i].goodExitStatusFileHandler);
    }
}

/*
generating_expected_outputs():
-----------------------------
Creates all expected output files for each test in the TestFileList struct. 
It iterates through each test, generates the expected output, and then closes
the file handlers.

arg1: parameters - The CommandLineArgs struct containing command line 
arguments such as the directory name to  store the expected output files.
arg2: testList - The TestFileList struct containing the tests to generate expected output for.

Returns: None
Errors: None
*/

void generating_expected_outputs(CommandLineArgs *parameters,
                                 TestFileList *testList) {
    make_test_directory(parameters->dir);
    for (int i = 0; i < testList->numTests; i++) {
        printf("Generating expected output for test %s\n",
               testList->tests[i].testID);
        create_expected_output_file(testList, parameters->dir, i);
        fill_expected_output(testList, i);
    }
    //Check if regen is required:
    for (int i = 0; i < testList->numTests; i++) {
        if (is_regen_needed(parameters->regen,
        parameters->jobFile, testList, i) == true) {
        printf("Generating expected output for test %s\n", testList->tests[i].testID);
        create_expected_output_file(testList, parameters->dir, i);
        fill_expected_output(testList, i);
        } else {
        continue;
        }
    }   

    close_file_handlers(testList);
}


/*
run_test_program():
-------------------
Runs the test program for a specific test. It redirects standard I/O to 
specific pipes and files, prepares the arguments for the test program,
 and then executes it.

arg1: testList - A pointer to the TestFileList struct that holds all 
the test information.
arg2: testProgram - The name of the test program to run.
arg3: index - The index of the specific test to run within the testList.

Returns: None
Errors: Exits with PROCESS_A_EXIT (ex) if execvp fails.
*/
void run_test_program(TestFileList *testList, char *testProgram, int index){
    //Open the input file for the test. 
    int testInputFileDescrip = open(testList->tests[index].testInputFileName,
    O_RDONLY);

    //Redirect the standard input to the test input file handler.
    dup2(testInputFileDescrip, STDIN_FILENO);
    //Redirect standardout, standard error to the write end of their pipes.
    dup2(testList->tests[index].standardOutCmp[WRITE_END], STDOUT_FILENO);
    dup2(testList->tests[index].standardErrorCmp[WRITE_END], STDERR_FILENO);

    //Close the input file and the write end of both pipes. 
    close(testInputFileDescrip);
    close(testList->tests[index].standardOutCmp[WRITE_END]);
    close(testList->tests[index].standardErrorCmp[WRITE_END]); 

    //Close the read end of both pipes, only writing is done in this process.
    close(testList->tests[index].standardOutCmp[READ_END]);
    close(testList->tests[index].standardErrorCmp[READ_END]);

    //Create the arguments for the command to to execute. 
    //The total argument size is the number of arguments + 2(accounting for 
    //the program name and the NULL value at the end of the array).
    int arraySize = testList->tests[index].testArgsCount + 2;
    char *arguments[arraySize];

    //Copy the program name into the array. 
    arguments[0] = testProgram;
    //Copy the command line arguments into the arguments array. 
    //The command line arguments are stored in the  testArgs variable.
    for (int k = 1; k < arraySize; k++){
        arguments[k] = testList->tests[index].testArgs[k-1]; 
    }

    //Exec the program with the arguments
    execvp(arguments[0], arguments);

    //ExiT if execvp fails. 
    _exit(PROCESS_A_EXIT); 

}

/*
compare_standard_out():
-----------------------
Compares the standard output of a test program with its expected standard
output. This is done my redirecting the standard input to the read end of 
the standardOutCmp and then running the 'cmp' program.

arg1: testList - A pointer to the TestFileList struct that holds all the 
test information.
arg2: index - The index of the specific test within the testList.
arg3: stdoutFileName - The file name containing the expected standard output.

Returns: None
Errors: Exits with PROCESS_B_EXIT (exit status 98) if execlp fails.
*/

void compare_standard_out(TestFileList *testList, int index,
char *stdoutFileName){

    //Redirect the standard input to the read end of the standardOutcmp pipe.
    dup2(testList->tests[index].standardOutCmp[READ_END], STDIN_FILENO);

    //Open a file descriptor for "/dev/null" for writing only.
    int devNull = open("/dev/null", O_WRONLY);
    //Redirect standard out and standard error to /dev/null.
    dup2(devNull, STDOUT_FILENO);
    dup2(devNull, STDERR_FILENO);

    //Close the file descriptors for the pipe ends, and /dev/null. 
    close(devNull);
    close(testList->tests[index].standardErrorCmp[READ_END]);
    close(testList->tests[index].standardErrorCmp[WRITE_END]);
    close(testList->tests[index].standardOutCmp[WRITE_END]);
    close(testList->tests[index].standardErrorCmp[READ_END]);

    //Use the execlp function to run the 'cmp' function.
    execlp("cmp", "cmp", stdoutFileName, NULL); 

    //Exit if unsuccessful
    _exit(PROCESS_B_EXIT);
}

/*
compare_standard_error():
-------------------------
Compares the standard error of a test program with its expected standard error.
It does this by running the 'cmp' program.

arg1: testList - A pointer to the TestFileList struct that holds all the test
information.
arg2: index - The index of the specific test within the testList.
arg3: stderrFileName - The file name containing the expected standard error.

Returns: None (void)
Errors: Exits with PROCESS_C_EXIT (exit status 99) if execlp fails.
*/
void compare_standard_error(TestFileList *testList, int index,
char *stderrFileName){

    //Redirect the standard input to the read end of the standErrorCmp pipe. 
    dup2(testList->tests[index].standardErrorCmp[READ_END], STDIN_FILENO);

    //Open the /dev/null file descriptor for writing.
    int devNull = open("/dev/null", O_WRONLY);

    //Redirect standard out and standard error to /dev/null.
    dup2(devNull, STDOUT_FILENO);
    dup2(devNull, STDERR_FILENO);
    
    //Close all file handlers: 
    close(devNull);
    close(testList->tests[index].standardErrorCmp[READ_END]);
    close(testList->tests[index].standardErrorCmp[WRITE_END]);
    close(testList->tests[index].standardOutCmp[WRITE_END]);
    close(testList->tests[index].standardErrorCmp[READ_END]);

    //Run the 'cmp' program through the execlp command. 
    execlp("cmp", "cmp", stderrFileName, NULL);

    //Exit if unsuccessful.
    _exit(PROCESS_C_EXIT);
}


/*
is_sigkilled():
----------------
Checks if any child process was killed by a signal (SIGKILL) by
iterating through an array of wait status and evaluating if a signal
killed the child process.

arg1: waitStatusArray - An array of wait statuses for the child processes.
arg2: numChildren - The number of child processes.

Returns: true if any child process was killed by a signal; otherwise, false.
Errors: None
*/
bool is_sigkilled(int *waitStatusArray, int numChildren){

    //Iterate through the wait status and check if any exited 
    //through a signal (SIGKILL):
    for (int i = 0; i < numChildren; i++){
        if (WIFSIGNALED(waitStatusArray[i])){
            return true;
        }
    }
    return false;
}

/*
is_exec_failed():
-----------------
Checks if any child process failed to execute the exec command based on its
exit status.

arg1: waitStatusArray - An array of wait statuses for the child processes.
arg2: numChildren - The number of child processes.
arg3: testId - The ID of the test being run.

Returns: true if any child process failed to execute; otherwise, false.
Errors: Prints an error message to stdout if exec fails.
*/
bool is_exec_failed(int *waitStatusArray, int numChildren, char *testId){
    //Check if each of the wait status of the child is the same
    //as the exit statu under the exec call for that child. 
    if (WEXITSTATUS(waitStatusArray[PROCESS_A_INDEX]) == PROCESS_A_EXIT ||
        WEXITSTATUS(waitStatusArray[PROCESS_B_INDEX]) == PROCESS_B_EXIT ||
        WEXITSTATUS(waitStatusArray[PROCESS_C_INDEX]) == PROCESS_C_EXIT){
        printf("Unable to execute job %s\n", testId);
        fflush(stdout);
        return true;
    }

    //If the execs of all child process succeeded, then return with false. 
    return false; 
}


/*
is_standard_out_error_matched():
--------------------------------
Checks if the standard output and standard error of a test match
the expected output and error.

arg1: waitStatusArray - An array of wait statuses for the child processes.
arg2: testId - The ID of the test being run.

Returns: true if both standard output and standard error match; otherwise, 
false.
Errors: Prints a status message to stdout depending on the result of the 
comparison.
*/
bool is_standard_out_error_matched(int *waitStatusArray, char *testId){
    
    bool standardOutMatched;
    bool standardErrorMatched; 

    int cmpStandardOut = WEXITSTATUS(waitStatusArray[PROCESS_B_INDEX]);
    int cmpStandardError = WEXITSTATUS(waitStatusArray[PROCESS_C_INDEX]);

    //Check if the status of process A is 0. 
    if (cmpStandardOut == 0){
        standardOutMatched = true; 
        printf("Job %s: Stdout matches\n", testId);
        fflush(stdout);
    }

    else if (cmpStandardOut == 1){
        standardOutMatched = false; 
        printf("Job %s: Stdout differs\n", testId);
        fflush(stdout);
    }

    if (cmpStandardError == 0){
        standardErrorMatched = true; 
        printf("Job %s: Stderr matches\n", testId);
        fflush(stdout);
    }

       else if (cmpStandardError == 1){
        standardErrorMatched = false; 
        printf("Job %s: Stderr differs\n", testId);
        fflush(stdout);
    }

    if (standardOutMatched && standardErrorMatched){
        return true;
    }
    
    else{
    return false;
    }

}



/*
is_exit_status_matched():
-------------------------
Checks if the exit status of a test matches the expected exit status.

arg1: waitStatusArray - An array of wait statuses for the child processes.
arg2: goodWaitStatus - The expected exit status.
arg3: testId - The ID of the test being run.

Returns: true if the exit status matches; otherwise, false.
Errors: Prints a status message to stdout.
*/
bool is_exit_status_matched(int *waitStatusArray, int goodWaitStatus,
    char *testId){

    int childExitStatus = WEXITSTATUS(waitStatusArray[PROCESS_A_INDEX]);
    if (childExitStatus == goodWaitStatus){
        printf("Job %s: Exit status matches\n", testId);
        fflush(stdout);
        return true;
    }
    
        else {
            printf("Job %s: Exit status differs\n", testId);
            fflush(stdout);
            return false;
        }

}


 

/*
running_tests_job_real():
-------------------------
Responsible for running each test job in the tests array of the testFileList
structure. It forks three child processes for each test. The first child
process runs the test program, the second child process compares the standard
output, and the third child process compares the standard error. 
The parent waits on the children to finish, and then checks the exit status.

arg1: testList - Pointer to the TestFileList struct containing the list of tests.
arg2: parameters - Pointer to CommandLineArgs struct with command-line arguments.

Returns: None
Errors: None
*/
void running_tests_job_real(TestFileList *testList, CommandLineArgs *parameters) {
    // Loop through the testFileList:
    char *testProgram = parameters->program;
    //Initialize the number of tests completed to 0 and passed.
    testList->testsCompleted = 0;
    testList->testsPassed = 0;

    for (int i = 0; i < testList->numTests; i++) {
        // Initialize the pipes for the standard out and standard error.
        pipe(testList->tests[i].standardOutCmp);
        pipe(testList->tests[i].standardErrorCmp);
        // Flush the standard out with the message:
        printf("Running job: %s\n", testList->tests[i].testID);

        // Initialize the pids for the children:
        int numChildren = 3;
        pid_t pidArray[numChildren];
        int waitStatusArray[numChildren];

        // Fork three times:
        for (int j = 0; j < numChildren; j++) {
            // Get the name of the expected output file for standard output and standard error.
            int buffer = 100;
            char stdoutFileName[100], stderrFileName[100];
            snprintf(stdoutFileName, buffer, "%s/%s.stdout", parameters->dir, testList->tests[i].testID);
            snprintf(stderrFileName, buffer, "%s/%s.stderr", parameters->dir, testList->tests[i].testID);

            // This will get the childPIDs and store them in an array.
            pid_t childPid = fork();
            // Code for child processes:
            if (childPid == 0) {
                // In process A: Run the test_program file the standard 
                //out/error pipes. 
                if (j == 0) {
                    run_test_program(testList,testProgram, i); 
                }
                // In process 2 (B): Compare the standard output. 
                if (j == 1) {
                    compare_standard_out(testList, i, stdoutFileName);
                }

                // In process 3 (C): Compare the standard error. 
                if (j == 2) {
                    compare_standard_error(testList, i, stderrFileName);
                }
            }

            pidArray[j] = childPid;
        }
   
        close(testList->tests[i].standardErrorCmp[READ_END]);
        close(testList->tests[i].standardErrorCmp[WRITE_END]);
        close(testList->tests[i].standardOutCmp[READ_END]);
        close(testList->tests[i].standardOutCmp[WRITE_END]);

        struct timespec processSleep;
        processSleep.tv_sec = 1;
        processSleep.tv_nsec = 500000000L;
        nanosleep(&processSleep, NULL);

        // Send SIGKILL to each child process
        for (int i = 0; i < numChildren; i++) {
            kill(pidArray[i], SIGKILL);
        }

        // In the parent process, iterate throught the pidArray, and wait for them:
        for (int i = 0; i < numChildren; i++) {
            int waitStatus;
            //Wait for the child to finish.
            waitpid(pidArray[i], &waitStatus, 0);
            //Put the wait stauts in the wait status array. 
            waitStatusArray[i] = waitStatus;
    
        }

        //Increment the number of jobs completed by one:
        testList->testsCompleted++;

        //Check if the test have failed thrugh a signal: 
        if (is_sigkilled(waitStatusArray, numChildren) == true){
            continue; 
        }

        //Check if any of the execs failed for the child processes.
        else if (is_exec_failed(waitStatusArray, numChildren,
        testList->tests[i].testID) == true){
            continue; 
        }

        bool standardOutErrorMatched = is_standard_out_error_matched(waitStatusArray,
        testList->tests[i].testID);
        bool exitStatusMatched = is_exit_status_matched(waitStatusArray,
        testList->tests[i].goodExitStatus, testList->tests[i].testID);

        if (standardOutErrorMatched == false || exitStatusMatched == false){
            continue;
        }
    
        else {
            testList->testsPassed++;
        }
    
    }
}


/*
report_on_test_jobs():
----------------------
Generates a report on the number of tests passed and completed. It also determines
the overall exit status of the program based on these numbers.

arg1: testList - Pointer to the TestFileList struct containing the list of tests.

Returns: None
Errors: Exits with exit status 0 if all tests passed; otherwise, exits with
exit status 7.
*/
void report_on_test_jobs(TestFileList *testList){
    //If at least one test has been completed, print the result: 
    if (testList->testsCompleted > 0){
        int testsPassed = testList->testsPassed;
        int testsCompleted = testList->testsCompleted;
        printf("testuqwordladder: %d of %d tests passes\n", testsPassed, testsCompleted);
    
        //If all test have passed exit with stautus 0: 
        if (testsPassed == testsCompleted){
            exit(OVERALL_TEST_RESULT_PASS);
        }

        else{
            exit(OVERALL_TEST_RESULT_FAIL);
        }
    }

}


int main(int argc, char *argv[]) {
    CommandLineArgs parameters;
    TestFileList testList;
    command_line_arguments(argc, argv, &parameters);
    job_specification_file(&parameters, &testList);
    generating_expected_outputs(&parameters, &testList);
    running_tests_job(&testList, &parameters); 
    running_tests_job_real(&testList, &parameters);
    report_on_test_jobs(&testList);

    
    return 0;
}



