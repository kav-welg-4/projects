  1 /*                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            2  * uqwordladder.c
  3  *      CSSE2310/7231 - Assignment One - 2023 - Semester Two
  4  *
  5  *      Written by Peter Sutton, p.sutton@uq.edu.au
  6  *      This version personalised for s4748290 Kavisha WELGAMA
  7  */
  8
  9 #include <stdio.h>
 10 #include <string.h>
 11 #include <stdlib.h>
 12 #include <ctype.h>
 13 #include <stdbool.h>
 14 #include <csse2310a1.h>
 15 #include <unistd.h>
 16 #include <getopt.h>
 17 #include <limits.h>
 18
 19 // The maximum length of any dictionary word can be assumed to be 50 chars
 20 #define MAX_DICTIONARY_WORD_LENGTH 50
 21
 22 // When reading dictionary words into a buffer, we need to allow space for
 23 // the word + a newline + a terminating null
 24 #define WORD_BUFFER_SIZE (MAX_DICTIONARY_WORD_LENGTH + 2)
 25
 26 // Default dictionary that we search
 27 #define DEFAULT_DICTIONARY "/usr/share/dict/words"
 28
 29 // Limits on length of word
 30 #define MIN_WORD_LENGTH 2
 31 #define MAX_WORD_LENGTH 9
 32 #define DEFAULT_WORD_LENGTH 4
 33
 34 // Command line option arguments
 35 #define START_ARG_TEXT "--start"
 36 #define END_ARG_TEXT "--end"
 37 #define MAX_ARG_TEXT "--limit"
 38 #define LEN_ARG_TEXT "--len"
 39 #define DICTIONARY_ARG_TEXT "--dictionary"
 40
 41 // Limit on the number of steps that will be permitted by default
 42 // (i.e. if not specified on the command line)
 43 #define DEFAULT_STEP_LIMIT 20
 44
 45 // Max value that will be accepted on the step limit (min value is the
 46 // length of the word.)
 47 #define MAX_STEP_LIMIT 50
 48
 49 // Usage error message
 50 #define USAGE_MESSAGE "Usage: uqwordladder [--start startWord] " \
 51         "[--end destWord] [--limit stepLimit] " \
 52         "[--len length] [--dictionary dictfilename]\n"
 53
 54 // Error messages output when command line arguments are invalid
 55 #define ERROR_MSG_INCONSISTENT \
 56         "uqwordladder: Word length conflict - lengths should be consistent\n"
 57 #define ERROR_MSG_BAD_LENGTH \
 58         "uqwordladder: Word lengths should be from 2 to 9 (inclusive)\n"
 59 #define ERROR_MSG_INVALID_WORD \
 60         "uqwordladder: Words should contain only letters\n"
 61 #define ERROR_MSG_SAME_WORDS \
 62         "uqwordladder: Start and end words must be different\n"
 63 #define ERROR_MSG_INVALID_STEP_LIMIT \
 64         "uqwordladder: Limit on steps must be word length to %d (inclusive)\n"
 65 #define ERROR_MSG_BAD_DICTIONARY "uqwordladder: File \"%s\" cannot be opened\n"
 66
 67 // Messages output during game play and on game over
 68 #define MSG_INVALID_WORD_LENGTH "Word should have %d characters - try again.\n"
 69 #define MSG_INVALID_WORD_CHARACTERS \
 70         "Word must contain only letters - try again.\n"
 71 #define MSG_INVALID_WORD_DIFFERENCE \
 72         "Word should differ by only one letter - try again.\n"
 73 #define MSG_WORD_REPEATED "You cannot repeat a previous word - try again.\n"
 74 #define MSG_WORD_NOT_IN_DICTIONARY \
 75         "Word cannot be found in dictionary - try again.\n"
 76 #define MSG_GAME_OVER_SUCCESS "You solved the ladder in %d steps.\n"
 77 #define MSG_GAME_OVER_RAN_OUT_OF_STEPS \
 78         "Game over - no more attempts remaining.\n"
 79 #define MSG_GAME_OVER_EOF "Game over.\n"
 80
 81 // Messages related to printing suggestions
 82 #define MSG_SUGGESTIONS_HEADER "Suggestions:-----------\n"
 83 #define MSG_SUGGESTIONS_FOOTER "-----End of Suggestions\n"
 84 #define MSG_SUGGESTIONS_NONE_FOUND "No suggestions found.\n"
 85
 86 // Enumerated type with our exit statuses
 87 typedef enum {
 88     OK = 0,
 89     DICTIONARY_ERROR = 3,
 90     GAME_OVER = 8,
 91     QUIT = 10,
 92     STEP_LIMIT_ERROR = 1,
 93     USAGE_ERROR = 4,
 94     WORD_ERROR = 18,
 95     WORD_LEN_CONSISTENCY_ERROR = 12,
 96     WORD_LEN_ERROR = 5,
 97     WORD_SAME_ERROR = 2
 98 } ExitStatus;
 99
100 // Enumerated type with our argument types - used for the getopt() version
101 // of command line argument parsing
102 typedef enum {
103     FROM_ARG = 1,
104     TO_ARG = 2,
105     MAX_ARG = 3,
106     LEN_ARG = 4,
107     DICTIONARY_ARG = 5
108 } ArgType;
109
110 // Structure type to hold our game parameters - obtained from the command line
111 typedef struct {
112     char* startWord;
113     char* endWord;
114     char* wordLenStr;
115     int wordLen;
116     char* stepLimitStr;
117     int stepLimit;
118     char* dictionaryFileName;
119 } GameParameters;
120
121 // Structure type to hold a list of words - used for the dictionary, as well
122 // as the list of entered words
123 typedef struct {
124     int numWords;
125     char** wordArray;
126 } WordList;
127
128 /* Function prototypes - see descriptions with the functions themselves */
129 GameParameters process_command_line(int argc, char* argv[]);
130 GameParameters process_command_line_getopt(int argc, char* argv[]);
131 int validate_word_lengths(const GameParameters param);
132 int validate_step_limit(const GameParameters param);
133 void validate_word_arguments(const GameParameters param);
134 void usage_error(void);
135 void word_length_error(void);
136 void word_length_consistency_error(void);
137 void same_word_error(void);
138 void step_limit_error(void);
139 void dictionary_error(const char* fileName);
140 bool is_string_positive_decimal(char* str);
141 void check_word_is_valid(const char* word);
142 WordList read_dictionary(const GameParameters param);
143 bool words_differ_by_one_char(const char* word1, const char* word2);
144 WordList add_word_to_list(WordList words, const char* word);
145 void free_word_list(WordList words);
146 char* convert_word_to_upper_case(char* word);
147 bool word_contains_only_letters(const char* word);
148 bool is_word_in_list(const char* word, WordList words);
149 char* read_line(void);
150 ExitStatus play_game(GameParameters param, WordList words);
151 bool check_attempt(const char* attempt, int wordLen, WordList validWords,
152         WordList previousSteps, const char* targetWord);
153 void print_suggestions(WordList previousWords, WordList validWords,
154         const char* targetWord);
155
156 /*****************************************************************************/
157 int main(int argc, char* argv[]) {
158     GameParameters gameDetails;
159     WordList validWords;
160
161     // Process the command line arguments - and get supplied game parameters.
162     // Will not return if arguments are invalid - prints message and exits.
163 #ifdef USE_GETOPT
164     gameDetails = process_command_line_getopt(argc, argv);
165 #else
166     gameDetails = process_command_line(argc, argv);
167 #endif
168     // Check the word lengths are valid/consistent and determine the length
169     gameDetails.wordLen = validate_word_lengths(gameDetails);
170
171     // Check the words are valid and different.
172     validate_word_arguments(gameDetails);
173
174     // Make sure the step limit is valid if specified
175     gameDetails.stepLimit = validate_step_limit(gameDetails);
176
177     // Read the dictionary into memory
178     validWords = read_dictionary(gameDetails);
179
180     // If one or other word is not specified, get and copy a random word from
181     // the dictionary. If a word is not coming from the dictionary we also
182     // strdup() so that we know these strings are always in dynamically
183     // allocated memory (makes freeing easier).
184     if (!gameDetails.startWord) {
185         gameDetails.startWord =
186                 strdup(get_uqwordladder_word(gameDetails.wordLen));
187     } else {
188         gameDetails.startWord = strdup(gameDetails.startWord);
189     }
190     if (!gameDetails.endWord) {
191         gameDetails.endWord =
192                 strdup(get_uqwordladder_word(gameDetails.wordLen));
193     } else {
194         gameDetails.endWord = strdup(gameDetails.endWord);
195     }
196
197     // Play the game and output the result
198     ExitStatus status = play_game(gameDetails, validWords);
199
200     // Tidy up and exit
201     free_word_list(validWords);
202     free(gameDetails.startWord);
203     free(gameDetails.endWord);
204     return status;
205 }
206
207 /*
208  * process_command_line()
209  *      Go over the supplied command line arguments, check their validity, and
210  *      if OK return the game parameters. (The start and end words, if given,
211  *      are converted to upper case.) If the command line is invalid, then
212  *      we print a usage error message and exit.
213  */
214 GameParameters process_command_line(int argc, char* argv[]) {
215     // No parameters to start with (these values will be updated with values
216     // from the command line, if specified)
217     GameParameters param = { .startWord = NULL, .endWord = NULL,
218             .wordLenStr = NULL, .stepLimitStr = NULL,
219             .dictionaryFileName = NULL };
220
221     // Skip over the program name argument
222     argc--;
223     argv++;
224
225     // Check for option arguments - we know they come in pairs with a value
226     // argument so only keep processing if there are at least 2 args left.
227     while (argc >= 2 && argv[0][0] == '-') {
228         if (strcmp(argv[0], START_ARG_TEXT) == 0 && !param.startWord) {
229             param.startWord = convert_word_to_upper_case(argv[1]);
230         } else if (strcmp(argv[0], END_ARG_TEXT) == 0 && !param.endWord) {
231             param.endWord = convert_word_to_upper_case(argv[1]);
232         } else if (strcmp(argv[0], LEN_ARG_TEXT) == 0 && !param.wordLenStr &&
233                 is_string_positive_decimal(argv[1])) {
234             param.wordLenStr = argv[1];
235         } else if (strcmp(argv[0], MAX_ARG_TEXT) == 0 && !param.stepLimitStr &&
236                 is_string_positive_decimal(argv[1])) {
237             param.stepLimitStr = argv[1];
238         } else if (strcmp(argv[0], DICTIONARY_ARG_TEXT) == 0
239                 && !param.dictionaryFileName) {
240             param.dictionaryFileName = argv[1];
241         } else {
242             // Unexpected argument (could be a repeated argument also)
243             usage_error();
244         }
245         // If we got here, we processed an option argument and value - skip
246         // over those, then return to the top of the loop to check for more
247         argc -= 2;
248         argv += 2;
249     }
250
251     // If any arguments now remain then this is a usage error
252     if (argc) {
253         usage_error();
254     }
255
256     return param;
257 }
258
259 /*
260  * process_command_line_getopt()
261  *      Go over the supplied command line arguments, check their validity, and
262  *      if OK return the game parameters. (The start and end words, if given,
263  *      are converted to upper case.) If the command line is invalid, then
264  *      we print a usage error message and exit.
265  *      This is a getopt version of the command line processing.
266  */
267 GameParameters process_command_line_getopt(int argc, char* argv[]) {
268     // No parameters to start with (these values will be updated with values
269     // from the command line, if specified)
270     GameParameters param = { .startWord = NULL, .endWord = NULL,
271             .wordLenStr = NULL, .stepLimitStr = NULL,
272             .dictionaryFileName = NULL };
273
274     // REF: Code based on the example in the getopt(3) man page
275     // Note the "+ 2"s here are to skip over the "--" part of the argument
276     // string because those constants are defined for the arg checking function
277     // above. If you were only using getopt() then you would define the
278     // constants differently, i.e. without the "--" prefix.
279     struct option longOptions[] = {
280             { START_ARG_TEXT + 2, required_argument, 0, FROM_ARG},
281             { END_ARG_TEXT + 2, required_argument, 0, TO_ARG},
282             { MAX_ARG_TEXT + 2, required_argument, 0, MAX_ARG},
283             { LEN_ARG_TEXT + 2, required_argument, 0, LEN_ARG},
284             { DICTIONARY_ARG_TEXT + 2, required_argument, 0, DICTIONARY_ARG},
285             { 0, 0, 0, 0}};
286     int optionIndex = 0;
287
288     while (true) {
289         // Get the next option argument. (":" prevents error message printing)
290         int opt = getopt_long(argc, argv, ":", longOptions, &optionIndex);
291         fflush(stdout);
292         if (opt == -1) { // Ran out of option arguments
293             break;
294         } else if (opt == FROM_ARG && !param.startWord) {
295             param.startWord = convert_word_to_upper_case(optarg);
296         } else if (opt == TO_ARG && !param.endWord) {
297             param.endWord = convert_word_to_upper_case(optarg);
298         } else if (opt == LEN_ARG && !param.wordLenStr &&
299                 is_string_positive_decimal(optarg)) {
300             param.wordLenStr = optarg;
301         } else if (opt == MAX_ARG && !param.stepLimitStr &&
302                 is_string_positive_decimal(optarg)) {
303             param.stepLimitStr = optarg;
304         } else if (opt == DICTIONARY_ARG && !param.dictionaryFileName) {
305             param.dictionaryFileName = optarg;
306         } else {
307             usage_error();
308         }
309     }
310
311     // If any arguments now remain, then this is a usage error.
312     if (optind < argc) {
313         usage_error();
314     }
315
316     return param;
317 }
318
319 /*
320  * validate_word_lengths()
321  *      Makes sure the word lengths given in the game parameters are valid and
322  *      consistent, otherwise we exit with an appropriate message. If the
323  *      lengths specified are correct and consistent then we return the word
324  *      length. If no lengths are specified in the game parameters then we
325  *      return the default length.
326  */
327 int validate_word_lengths(const GameParameters param) {
328     int len = DEFAULT_WORD_LENGTH;
329     // Get the wordLength from command line, if given. We already know the
330     // wordLenStr element is a positive decimal integer.
331     if (param.wordLenStr) {
332         len = atoi(param.wordLenStr);
333         if (strlen(param.wordLenStr) >= (sizeof(int) * 2)) {
334             // The word length string is way too long and could have overflowed
335             // so we just set the length to INT_MAX. sizeof(int)*2 is the
336             // number of hex digits in an integer so we can be sure if the num
337             // of decimal digits is this or bigger then we have a problem.
338             len = INT_MAX;
339         }
340     }
341
342     // Check for word length consistency if more than one of the starter word,
343     // target word and word length are specified.
344     if (param.startWord && param.endWord &&
345             strlen(param.startWord) != strlen(param.endWord)) {
346         word_length_consistency_error();
347     }
348     if (param.startWord && param.wordLenStr &&
349             strlen(param.startWord) != len) {
350         word_length_consistency_error();
351         B
352     }
353     if (param.endWord && param.wordLenStr && strlen(param.endWord) != len) {
354         word_length_consistency_error();
355     }
356
357     // Lengths are consistent. Make sure we have the value from whatever
358     // field has been specified.
359     if (!param.wordLenStr) {
360         if (param.startWord) {
361             len = strlen(param.startWord);
362         } else if (param.endWord) {
363             len = strlen(param.endWord);
364         }
365     } // else have already set the 'len' variable to this length
366
367     // Check the length is valid
368     if (len < MIN_WORD_LENGTH || len > MAX_WORD_LENGTH) {
369         word_length_error();
370     }
371
372     // Lengths consistent and valid - return the word length for this game
373     return len;
374 }
375
376 /*
377  * validate_step_limit()
378  *      Makes sure the step limit in the game parameters (param)
379  *      is valid if specified - it must be between the word length
380  *      and MAX_STEP_LIMIT. If not, we will print an appropriate
381  *      error message and exit. We return the step limit (which
382  *      will be default limit if it wasn't specified on the command
383  *      line).
384  */
385 int validate_step_limit(const GameParameters param) {
386     if (param.stepLimitStr) {
387         // We already know the string must represent a positive decimal integer
388         // so we need to check the range. There is a possibility that atoi()
389         // overflows, so we also check that the length of the string is as
390         // expected.
391         int limit = atoi(param.stepLimitStr);
392         if (strlen(param.stepLimitStr) > 2 ||
393                 limit < param.wordLen || limit > MAX_STEP_LIMIT) {
394             step_limit_error(); // will not return
395         }
396         return limit;
397     } else {
398         return DEFAULT_STEP_LIMIT;
399     }
400 }
401
402 /*
403  * validate_word_arguments()
404  *      Check the initial and target words provided on the command line
405  *      (from param) to make sure the words are valid and (if both are
406  *      given) different. Exit with an error message if not.
407  */
408 void validate_word_arguments(const GameParameters param) {
409     // If a start or end word has been given on the command line then
410     // check they are valid
411     if (param.startWord) {
412         check_word_is_valid(param.startWord); // may not return
413     }
414     if (param.endWord) {
