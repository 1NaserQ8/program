#ifndef QUESTIONS_H
#define QUESTIONS_H

#define MAX_QUESTION_LEN 256
#define MAX_OPTION_LEN 128

typedef struct {
    char question[MAX_QUESTION_LEN];
    char optionA[MAX_OPTION_LEN];
    char optionB[MAX_OPTION_LEN];
    char optionC[MAX_OPTION_LEN];
    char optionD[MAX_OPTION_LEN];
    char correct_answer;
} Question;

Question *load_questions(const char *filename, int *count);
void free_questions(Question *questions);
int check_answer(const Question *q, char ans);

#endif
