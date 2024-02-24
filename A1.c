#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
//#include <csse2310a1.h>

/*Define macros for errror messages, default values, and exit messages */  

//For Invalid Command Line Argument: Command Line arguments have typos in them: 
#define INVALID_COMMAND_LINE_ARGUMENT_4   "Usage: uqwordladder [--start startWord] [--end destWord] [--limit stepLimit] 106"\
"[--len length] [--dictionary dictfilename]"
#define EXIT_STATUS_4 4 

//For length validity checking: 
#define MIN__WORD_LENGTH 2
#define MAX__WORD_LENGTH 9 
#define DEFAULT_WORD_LENGTH 4
#define WORD_LENGTH_CONFLICT_12 "uqwordladder: Word length conflict - lengths should be consistent"
#define EXIT_STATUS_12 12 //Exit status for conflicting word lengths (can be used to compare just startWord and destWord)
#define WORD_LENGTH_OUTSIDE_RANGE_5 "uqwordladder: Word length should be from 2 to 9 (inclusive)"
#define EXIT_STATUS_5 5 //Exit status if any of the given word arguments or length is not within 2 to 9 inclusve. 

//For Command Line Word Checking (for invalid characters, and start, end word differences):
#define INVALID_CHARACTER_INPUT_18 "uqwordladder: Word should contain only letters"
#define EXIT_STATUS_18 18 //Exit status for invalid character input.
#define SAME_START_END_WORDS_2 "uqwordladder: Start and end words must be different"
#define EXIT_STATUS_2 2

//For steplimit length: 
#define MAX_LIMIT_LENGTH 50
#define DEFAULT_LIMIT_LENGTH 20 
#define LIMIT_EXCEEDED_LENGTH_1 "uqwordladder: Limit on steps must be word length to 50 (inclusive)"
#define EXIT_STATUS_1 1


#define DICTIONARY_PATH "/usr/share/dict/words"
#define INVALID_DICTIONARY_FILE_3 "uqwordladder: File (filaname) cannot be opened"
#define EXIT_STATUS_3 3


/*Define a typedef struct to store values from the command line arguments. */
typedef struct {
    char *startword; 
    char *destword; 
    char *dictionary; 
    char **dictionaryElement; //Dinctionary is a collection/list of multiple strings.
    int len; 
    int limit; 
} cmdargs; 




/*The function will take in a string (the next string after --len and --limit) and return a.
Will return 1 if the string next to --len and --limit was not actually a number.
If it is a number, but not positive, then it will return 1.
If positive, the function will also check if --len and --limit is within the minimum and maximum range????
Will retrun 0 if the string was actually a number 1.
*/

bool valid_integer(char*str, int count){
    if (count >= 1){ //If the ount if already equal to 1, then the user has already supplied a value for the command line argument.
        return false;  //Return 1, which can then be used to run exit_status_4
    }

    int digit; 
    int string_length =  strlen(str);
    if (string_length ==0){
        return false;  //If the string length is 0, then the user has not supplied anything.
    }
    //Iterate through the string to determine whether the string contains any non-digit values
    for (int i = 0; i < string_length; i++){
        if (isdigit(str[i])==0){
            return false; // If the value is not a digit, then return 1, which then returns exit 0. 
        }
        else {
            digit = str[i]; 
            if (digit <= 0 ){
                return false; //If the digit is negative. 
            }
        }

    }

    //Now the string has been verified as only containing number digits. Now, convert the valid string into an integer. 
   /* int integer = atoi(str); 
    if (integer <= 0){  //Now that the string has been converted into an integer, check if positive. 
        return false; //If the digit is not positive, return 0, which signals to the argumnet function to exit with status 4. 
    }*/

    return true; 
}



void command_line_arguments(int argc, char *argv[], cmdargs *arguments)
{
    arguments->startword = NULL;
    arguments->destword = NULL;
    arguments->dictionary = NULL;
    arguments->len = -1;
    arguments->limit = -1;

    // Create counter variables too see how many times each command_line argument has been supplied:
    int startWord_count = 0;
    int destWord_count = 0;
    int len_count = 0;
    int limit_count = 0;
    int dict_count = 0;

    // Declare variables to handle what the functions for only_letters and valid_integer will return.
    int len_check;
    int limit_check;

    // If statement to check for the command line arguments: Iterate through the argc.
    for (int i = 0; i < argc - 1; i++)
    {
        // Check for the "--start" command line argument.
        if (strcmp(argv[i], "--start") == 0)
        {
            if (startWord_count >= 1)
            { // If the count is already equal to 1, then the user has alread supplied a value for the command line argument.
                fprintf(stderr, "%s\n", INVALID_COMMAND_LINE_ARGUMENT_4);
                exit(EXIT_STATUS_4);
            }

            else
            {
                startWord_count++;
                arguments->startword = (char *)malloc(sizeof(char) * (strlen(argv[i + 1]) + 1));
                strcpy(arguments->startword, argv[i + 1]);
            }
        }

        // Check for "--end" command line argument.
        else if (strcmp(argv[i], "--end") == 0)
        {

            destWord_count++;
            arguments->destword = (char *)malloc(sizeof(char) * (strlen(argv[i + 1]) + 1));
            strcpy(arguments->destword, argv[i + 1]);
        }

        // For "--len" command line argument checking.
        else if (strcmp(argv[i], "--len") == 0)
        {
            len_check = valid_integer(argv[i + 1], len_count); // Run the only_letters function to check if the start word is valid.

            // Check the output of the only_letters function. If output is 1, then run exit_status_4.

            if (len_check == false)
            {
                fprintf(stderr, "%s\n", INVALID_COMMAND_LINE_ARGUMENT_4);
                exit(EXIT_STATUS_4);
            }

            else if (len_check == true)
            {
                arguments->len = atoi(argv[i + 1]);
                len_count++;
            }
        }

        // For "--limit" command line argument checking.
        else if (strcmp(argv[i], "--limit") == 0)
        {
            limit_check = valid_integer(argv[i + 1], limit_count); // Run the only_letters function to check if the start word is valid.

            // Check the output of the only_letters function. If output is 1, then run exit_status_4.

            if (limit_check == false)
            {
                fprintf(stderr, "%s\n", INVALID_COMMAND_LINE_ARGUMENT_4);
                exit(EXIT_STATUS_4);
            }

            else if (limit_check == true)
            {
                limit_count++;
                arguments->limit = atoi(argv[i + 1]);
            }
        }

        // For "--dictionary" command line argument checking.
        else if (strcmp(argv[i], "--dictionary") == 0)
        {
            dict_count++;
            arguments->dictionary = (char *)malloc(sizeof(char) * (strlen(argv[i + 1]) + 1));
            strcpy(arguments->dictionary, argv[i + 1]);
            // The dictionary file name validity will be checked later.
        }
    }
}

/*
Functions to check word length validity: 
1) void word_length_valid(char*str) ~ Checks for string length exceeded
2)void length_valid(int length) ~Checks for length integer being too large
3) void string_length_comparison(char*str, int length) ~ Compares two strings
4) void string_string_comparison(char*string1, char*string2) ~Compares a string and an integer
5) void length_validity_check(cmdargs *arguments) ~ Checks if arguments supplied, and resolves for length conflicts.  
*/
void word_length_valid(char*str){
    /* Takes in a string (from the startWord or destWord) and determines if they are outside
    of the range of 2 to 9 inclusive. 
    Does not return anything, but exits if the word length is outside the range    
    */
    if ( strlen(str) < 2 || strlen(str) > 9){
        fprintf(stderr,"%s\n", WORD_LENGTH_OUTSIDE_RANGE_5);
        exit(EXIT_STATUS_5);
    }
}


void length_valid(int length){
    /* 
    
    */
    if (length < 2 || length > 9){
        fprintf(stderr,"%s\n", WORD_LENGTH_OUTSIDE_RANGE_5);
        exit(EXIT_STATUS_5);
    }

}

void string_length_comparison(char*str, int length){
    if (strlen(str) != length){
        fprintf(stderr,"%s\n", WORD_LENGTH_CONFLICT_12);
        exit(EXIT_STATUS_12);
    }
}

void string_string_comparison(char*string1, char*string2){
    if (strlen(string1)!= strlen(string2)){
        fprintf(stderr,"%s\n", WORD_LENGTH_CONFLICT_12);
        exit(EXIT_STATUS_12);
    }
}

void length_validity_check(cmdargs *arguments)
{

    // Complete the comparison between the lengths of the words if they are given.
    if (arguments->len != -1 && arguments->startword != NULL)
    {
        string_length_comparison(arguments->startword, arguments->len);
    }
    if (arguments->len != -1 && arguments->destword != NULL)
    {
        string_length_comparison(arguments->destword, arguments->len);
    }
    if (arguments->destword != NULL && arguments->startword != NULL)
    {
        string_string_comparison(arguments->destword, arguments->startword);
    }

    // If len not given, check if start or dest given, then set len to the word length of given word.
    if (arguments->len == -1)
    {
        if (arguments->startword != NULL)
        {
            arguments->len = strlen(arguments->startword);
        }
        else if (arguments->destword != NULL)
        {
            arguments->len = strlen(arguments->destword);
        }
        else if (arguments->startword == NULL && arguments->destword == NULL)
        {
            arguments->len = DEFAULT_WORD_LENGTH;
        }
    }

    // Check if the values are outside of range 2 to 9 inclusive.
    if (arguments->len != -1)
    {
        length_valid(arguments->len);
    }
    if (arguments->startword != NULL)
    {
        word_length_valid(arguments->startword);
    }
    if (arguments->destword != NULL)
    {
        word_length_valid(arguments->destword);
    }
}

/* 
For Command Line Word Checking: Dealing with startWord and desWord. 
1) void only_letter(char*string) ~ Checks if the string contains only letters
3) void same_words_check(char*string1, char*string2) ~ Compares two strings to see if they are the same.
2) void command_line_word_checking(cmdargs argument) ~ Checks if the startword and destword are valid. 
*/

void only_letters(char*string){
   int stringLength = strlen(string);  
    for (int i = 0; i < stringLength; i++){
        if (isalpha(string[i])==0){
           fprintf(stderr,"%s\n", INVALID_CHARACTER_INPUT_18);
           exit(EXIT_STATUS_18); 
        }
    }
}

void same_words_check(char*string1, char*string2){
    if (strcmp(string1, string2) == 0 ){
        fprintf(stderr, "%s/n", SAME_START_END_WORDS_2);
        exit(EXIT_STATUS_2);            
    }
}

void convert_to_uppercase(char *string) {
    int i = 0;
    while (string[i]) {
        string[i] = toupper(string[i]);
        i++;
    }
}



void command_line_word_checking(cmdargs *arguments)
    {
    //Check validity if startword or destword is given. Convert to uppercase if valid.
    if (arguments->startword != NULL)
    {
        only_letters(arguments->startword);
        convert_to_uppercase(arguments->startword);
    }

    if (arguments->destword != NULL)
    {
        only_letters(arguments->destword);
        convert_to_uppercase(arguments->destword);
    }

    //Check if startword and destword are the same.
    if (arguments->startword != NULL && arguments->destword != NULL)
    {
        same_words_check(arguments->startword, arguments->destword);
    }

    //If startword or destword is not given, then generate a new word based on the saved word length (len)
    if (arguments->startword == NULL)
    {
        arguments->startword = strdup(get_uqwordladder_word(arguments->len)); 
        convert_to_uppercase(arguments->startword);
    }

    if (arguments->destword == NULL)
    {
        arguments->destword = strdup(get_uqwordladder_word(arguments->len));
        convert_to_uppercase(arguments->destword);
    }
}


/*
Step Limit Checking: 
Checks whether the supplied stepLimit is between word length
and the maximum value of 50. 
Sets the default step limit to 20 if no value was provided 
by the user. 
*/

void step_limits_check(cmdargs *arguments){
    //Check whether steplimit is supplied. 
    if (arguments->limit == -1){
        arguments->limit = DEFAULT_LIMIT_LENGTH; 
    }

    else if (arguments->limit != -1){
        if (arguments->limit < arguments->len || arguments->limit > MAX_LIMIT_LENGTH){
            fprintf(stderr, "%s\n", LIMIT_EXCEEDED_LENGTH_1);
            exit(EXIT_STATUS_1);
        }
    }
}


/*
To check whether the dictionary file supplied by the user 
(if not the default dictionary) is  valid. 

1) void verifiy_dict_path(char *dictionaryFile) 
    ~Checks if the dictionary file path is valid.
2) void dictionary_file_check(cmdargs *arguments)
    ~Assigns the default dicitionary or checks for given path
    for validity. 
*/


void verifiy_dict_path(char *dictionaryFile){
    //Declare the file hanlde, and open the file in read mode. 
    FILE *fileHandle; 

    if ((fileHandle = fopen(dictionaryFile, "r")) == NULL){
        fprintf(stderr, "uqwordladder: File \"%s\" cannot be opened", dictionaryFile);
        exit(EXIT_STATUS_3);
    }


}


void dictionary_file_check(cmdargs *arguments){
    
    //If no dictiionary path supplied, set dictionary to default path. 
    if (arguments->dictionary == NULL){
        arguments->dictionary = strdup(DICTIONARY_PATH);
    }

    else if (arguments->dictionary != NULL){
        verifiy_dictionary_path(arguments->dictionary);
    }


}


/*
Program Operation: 


void program_operation_word_setting(cmdargs argument){
//If none of one of the words have been given, then generate new ones based on len. 


}


void program_operation_user_plays(cmdargs argument){



}





Function to extract the dictionary from the struct and check it. If no dictionary is given, then set the dictionary to be the default. 
Extract the dicionary 
Return the dictionary file name. 

char *dictionary_check_set(char *dictionary_file){
      
    return 0; 
}
*/





int main (int argc, char *argv[]){
    cmdargs arguments; 

    command_line_arguments(argc, argv, &arguments); //Run the command line arguments function to check for validity.
    //Then run arguments into each codeline. 
    length_validity_check(&arguments);
    command_line_word_checking(&arguments);
    step_limits_check(&arguments);
    dictionary_file_check(&arguments);
    free(arguments.startword);
    free(arguments.destword);



    //Free all the arrays: 
    //Free struct: 
    //free
    
    return 0; 
}