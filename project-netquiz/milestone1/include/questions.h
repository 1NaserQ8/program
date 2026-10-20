#ifndef QUESTIONS_H
#define QUESTIONS_H

//Abbas: Maximum length for question and options
#define MAX_QUESTION_LEN 256
#define MAX_OPTION_LEN 128

//Abbas: Question structure holding text, options, and correct answer
typedef struct {
    char question[MAX_QUESTION_LEN];
    char optionA[MAX_OPTION_LEN];
    char optionB[MAX_OPTION_LEN];
    char optionC[MAX_OPTION_LEN];
    char optionD[MAX_OPTION_LEN];
    char correct_answer;
} Question;

//Abbas: Load questions from file and return dynamically allocated array 
Question *load_questions(const char *filename, int *count);

//Abbas: Free dynamically allocated questions array
void free_questions(Question *questions);

//Abbas: Check if provided answer matches correct answer 
int check_answer(const Question *q, char ans);

#endif
