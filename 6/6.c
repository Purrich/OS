#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define MAX_ITEM_LENGTH 50

int withSleep = 0;

int usersCount = 0;

struct User {
    char Name[MAX_ITEM_LENGTH];
    char Password[MAX_ITEM_LENGTH];
};

struct User Users[50];

char* fileName = NULL;
char* lckFileName = NULL;

void waitForEditPermission()
{
    while (access(lckFileName, F_OK) == 0) {
        printf("waiting\n");
    }
}

void createLckFile()
{
    FILE* fp = fopen(lckFileName, "w");

    fprintf(fp, "%d\n", getpid());
    fclose(fp);
}
void removeLckFile()
{
    remove(lckFileName);
}

char* getUserName(char* line)
{
    return strtok(line, " ");
}
char* getPassword(char* line)
{
    return strtok(NULL, " \n");
}

void buildNewUsers(char* userName, char* newPassword)
{
    FILE* ifp = fopen(fileName, "r");

    int i = 0;
    int maxLineLength = MAX_ITEM_LENGTH * 2 + 1;
    char* line = malloc(maxLineLength);

    while (fgets(line, maxLineLength, ifp) != NULL) {
        char* currentUserName = getUserName(line);
        char* currentPassword = getPassword(line);

        strcpy(Users[i].Name, currentUserName);

        if (strcmp(currentUserName, userName) == 0)
            strcpy(Users[i].Password, newPassword);
        else
            strcpy(Users[i].Password, currentPassword);

        i++;
    }
    usersCount = i;

    fclose(ifp);

    free(line);
}

void updateUsers()
{
    FILE* ofp = fopen(fileName, "w");

    int i = 0;
    for (i = 0; i < usersCount; i++) {
        fprintf(ofp, "%s %s\n", Users[i].Name, Users[i].Password);
    }

    if (withSleep == 1)
        sleep(20);

    fclose(ofp);
}

int updatePassword(char* userName, char* newPassword)
{
    waitForEditPermission();

    createLckFile();

    buildNewUsers(userName, newPassword);
    updateUsers();

    removeLckFile();

    return 0;
}

void* initFileNames(char* sourceFileName)
{
    fileName = malloc(strlen(sourceFileName));
    strcpy(fileName, sourceFileName);

    char* fileNameWithoutExtension = strtok(sourceFileName, ".");

    lckFileName = malloc(strlen(fileNameWithoutExtension) + 4);

    strcpy(lckFileName, fileNameWithoutExtension);
    strcat(lckFileName, ".lck");
}

int main(int argc, char* argv[])
{
    if (argc != 4 && argc != 5) {
        printf("Error: invalid arguments!\n");
        printf("To start, print './6 <file_name> <user_name> <user_password> [sleep]'\n");
        return 1;
    }

    initFileNames(argv[1]);

    if (argc == 5 && strcmp(argv[4], "sleep") == 0)
        withSleep = 1;

    return updatePassword(argv[2], argv[3]);
}