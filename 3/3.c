#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <regex.h>
#include <stdlib.h>

#define MAX_INT_LENGTH 10

struct Node {
    int Value;
    struct Node* Next;
};
struct Node* First = NULL;

char* numbersRegexText = "[0-9]+";

struct Node* AddNode(int value)
{
    if (First == NULL) {
        First = malloc(sizeof(struct Node));

        First->Value = value;
        First->Next = NULL;

        return First;
    }

    struct Node* node = malloc(sizeof(struct Node));

    if (First->Value > value) {
        node->Next = First->Next;
        node->Value = First->Value;

        First->Next = node;
        First->Value = value;

        return node;
    }

    struct Node* currentNode = First;

    while (currentNode != NULL) {
        if (currentNode->Next == NULL || currentNode->Value <= value && currentNode->Next->Value >= value) {
            node->Next = currentNode->Next;
            node->Value = value;

            currentNode->Next = node;

            break;
        }
        currentNode = currentNode->Next;
    }

    return node;
}

void DestructNodes()
{
    struct Node* currentNode = First;
    struct Node* nextNode;

    while (currentNode != NULL) {
        nextNode = currentNode->Next;
        free(currentNode);
        currentNode = nextNode;
    }
}

FILE* tryOpenFile(char* fileName, char* permissions)
{
    FILE* fp;
    if ((fp = fopen(fileName, permissions)) == NULL) {
        printf("Error: cannot open file '%s'.\n", fileName);
        exit(1);
    }
    return fp;
}

void tryCompileRegex(regex_t* r)
{
    if (regcomp(r, numbersRegexText, REG_EXTENDED) != 0) {
        printf("Error while compiling regex '%s'\n", numbersRegexText);
        exit(1);
    }
}

int getMatchLength(regmatch_t match)
{
    return match.rm_eo - match.rm_so;
}

void parseLine(char* line)
{
    regex_t r;
    tryCompileRegex(&r);

    regmatch_t currentMatch;

    int matchesCountInLoop = 1;

    while (regexec(&r, line, matchesCountInLoop, &currentMatch, 0) == 0) {
        int currentMatchLength = getMatchLength(currentMatch);

        if (currentMatchLength < MAX_INT_LENGTH) {
            char* buffer = malloc(currentMatchLength);
            memcpy(buffer, line + currentMatch.rm_so, currentMatchLength);

            AddNode(atoi(buffer));

            free(buffer);
        }
        line += currentMatch.rm_eo;
    }
}

void parseFile(char* fileName)
{
    FILE* fp = tryOpenFile(fileName, "r");

    char* currentLine = NULL;
    size_t currentSize = 0;

    while ((getline(&currentLine, &currentSize, fp) != -1)) {
        parseLine(currentLine);

        currentLine = NULL;
        currentSize = 0;
    }

    fclose(fp);
}

void writeNumbersToFile(char* outputFileName)
{
    FILE* fp = tryOpenFile(outputFileName, "w");

    struct Node* currentNode = First;

    while (currentNode != NULL) {
        fprintf(fp, "%d\n", currentNode->Value);

        currentNode = currentNode->Next;
    }

    fclose(fp);
}

int main(int argc, char* argv[])
{
    if (argc < 3) {
        printf("Error: invalid arguments!\n");
        printf("To start, print './3 <file_name1> [<file_name2> ...] <output_file_name>'\n");
        return 1;
    }

    int i = 1;
    for (i = 1; i < argc - 1; i++) {
        parseFile(argv[i]);
    }

    writeNumbersToFile(argv[argc - 1]);

    DestructNodes();

    return 0;
}