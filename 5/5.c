#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <stdbool.h>

#define MAX_PROGRAM_NAME_LENGTH 50
#define MAX_ARGUMENTS_COUNT 20
#define MAX_ARGUMENT_LENGTH 20
#define MAX_PROGRAMS_COUNT 50
#define MAX_PROCESSES_COUNT 50

#define MAX_FAIL_COUNT 50

char* configFileName;

int programsCount = 0;
int processesCount = 0;

enum RunType { WAIT, RESPAWN };

struct Program {
    char Name[MAX_PROGRAM_NAME_LENGTH];
    char* Arguments[MAX_ARGUMENTS_COUNT];
    enum RunType Type;
};

struct Program Programs[MAX_PROGRAMS_COUNT];
pid_t Processes[MAX_PROCESSES_COUNT];

enum RunType parseRunType(char* runType)
{
    if (strcmp(runType, "wait") == 0)
        return WAIT;
    if (strcmp(runType, "respawn") == 0)
        return RESPAWN;

    printf("Error: unsupported RunType '%s'.\n", runType);
    exit(1);
}

int parseConfigFile()
{
    FILE* fp = fopen(configFileName, "r");

    char* currentLine = NULL;
    size_t currentSize = 0;

    int programsCounter = 0;

    while ((getline(&currentLine, &currentSize, fp) != -1)) {
        char* token = strtok(currentLine, " ");

        strcpy(Programs[programsCounter].Name, token);
        int i = 0;

        while (1) {
            char* nextToken = strtok(NULL, " ");

            if (nextToken == NULL) {
                nextToken = strtok(token, "\n");
                Programs[programsCounter].Type = parseRunType(nextToken);
                break;
            }

            Programs[programsCounter].Arguments[i] = malloc(MAX_ARGUMENT_LENGTH);
            strcpy(Programs[programsCounter].Arguments[i], token);

            token = nextToken;
            i++;
        }

        programsCounter++;
    }

    programsCount = programsCounter;
    fclose(fp);
}

char* buildPidFileName(int programNumber)
{
    char* pidFileName = malloc(strlen(Programs[programNumber].Name) + 9);

    strcpy(pidFileName, "/tmp/");
    strcat(pidFileName, Programs[programNumber].Name);
    strcat(pidFileName, ".pid");

    return pidFileName;
}

void savePidFile(pid_t pid, int programNumber)
{
    char* pidFileName = buildPidFileName(programNumber);

    FILE* fp = fopen(pidFileName, "w");

    fwrite(&pid, sizeof(pid_t), 1, fp);

    fclose(fp);
}

void removePidFile(int programNumber)
{
    char* pidFileName = buildPidFileName(programNumber);

    remove(pidFileName);
}

void freeArguments(int programNumber)
{
    int argNumber = 0;
    while (Programs[programNumber].Arguments[argNumber] != NULL) {
        free(Programs[programNumber].Arguments[argNumber]);
        argNumber++;
    }
}

void forkProcess(int programNumber)
{
    pid_t cpid = fork();

    switch (cpid) {
    case -1:
        printf("Error: cannot fork process %d\n", programNumber);
        break;
    case 0:
        execvp(Programs[programNumber].Name, Programs[programNumber].Arguments);
        exit(0);
    default:
        Processes[programNumber] = cpid;
        processesCount++;
        savePidFile(cpid, programNumber);
    }
}

int getProgramNumberByPid(pid_t pid)
{
    int i = 0;
    for (i = 0; i < programsCount; i++) {
        if (Processes[i] == pid) {
            break;
        }
    }
    return i;
}

void waitTillFinished()
{
    while (processesCount > 0) {
        pid_t pid = pid = waitpid(WAIT_ANY, NULL, 0);

        int programNumber = getProgramNumberByPid(pid);

        if (Programs[programNumber].Type == RESPAWN) {
            forkProcess(programNumber);
            continue;
        }

        freeArguments(programNumber);
        removePidFile(programNumber);

        Processes[programNumber] = 0;

        processesCount--;
    }
}

void manageProcesses()
{
    int i;

    for (i = 0; i < programsCount; i++) {
        forkProcess(i);
    }

    waitTillFinished();
}

void killProcesses()
{
    int i;
    for (i = 0; i < processesCount; i++) {
        if (Processes[i] > 0)
            kill(Processes[i], SIGKILL);
    }
}

void handleHUP()
{
    killProcesses();

    parseConfigFile();
    manageProcesses();
}

int main(int argc, char* argv[])
{
    if (argc < 2) {
        printf("Error: invalid arguments!\n");
        printf("To start, print './5 <config_file_name>'\n");
        return 1;
    }

    configFileName = argv[1];

    signal(SIGTSTP, handleHUP);

    parseConfigFile();
    manageProcesses();

    return 0;
}