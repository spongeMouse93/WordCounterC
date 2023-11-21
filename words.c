#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <ctype.h>

#define INIT_CAPACITY 1000
#define MAX_WORD_LENGTH 256

struct HashMap {
    char word[MAX_WORD_LENGTH];
    int freq;
};

bool isWordCharacter(char c) {
    return isalpha(c) || c == '\'' || c == '-';
}

void processWord(const char *word, struct HashMap *wordFreq, int *wordFreqSize) {
    bool found = false;
    for (int j = 0; j < *wordFreqSize; j++)
        if (strcmp(word, wordFreq[j].word) == 0){
            wordFreq[j].freq++;
            found = true;
            break;
        }
    if (!found){
        if (*wordFreqSize >= INIT_CAPACITY){
            *wordFreqSize *= 2;
            wordFreq = realloc(wordFreq, (*wordFreqSize) * sizeof(struct HashMap));
            if (wordFreq == NULL){
                perror("realloc");
                exit(1);
            }
        }
        strncpy(wordFreq[*wordFreqSize].word, word, MAX_WORD_LENGTH);
        wordFreq[*wordFreqSize].freq = 1;
        (*wordFreqSize)++;
    }
}

void processFile(const char *filename, struct HashMap *wordFreq, int *wordFreqSize) {
    int fd = open(filename, O_RDONLY);
    if (fd == -1){
        perror("open");
        return;
    }
    char buffer[MAX_WORD_LENGTH];
    ssize_t bytesRead;
    char word[MAX_WORD_LENGTH];
    int wordLength = 0;
    while ((bytesRead = read(fd, buffer, sizeof(buffer) - 1)) > 0){
        buffer[bytesRead] = '\0';
        for (int i = 0; i < bytesRead; i++){
            char c = buffer[i];
            if (isWordCharacter(c)){
                word[wordLength] = c;
                wordLength++;
                if (c == '-' && i + 1 < bytesRead && buffer[i + 1] == '-'){
                    if (wordLength > 0){
                        word[wordLength - 1] = '\0';
                        processWord(word, wordFreq, wordFreqSize);
                        wordLength = 0;
                    }
                    i++;
                }
            }else if (wordLength > 0){
                word[wordLength] = '\0';
                processWord(word, wordFreq, wordFreqSize);
                wordLength = 0;
            }
        }
    }
    if (wordLength > 0){
        word[wordLength] = '\0';
        processWord(word, wordFreq, wordFreqSize);
    }
    close(fd);
}

void processDirectory(const char *dirname, struct HashMap *wordFreq, int *wordFreqSize){
    DIR *dir = opendir(dirname);
    if (dir == NULL){
        perror("opendir");
        return;
    }
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
        if (entry->d_type == DT_REG){
            char filepath[1024];
            snprintf(filepath, sizeof(filepath), "%s/%s", dirname, entry->d_name);
            processFile(filepath, wordFreq, wordFreqSize);
        }else if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0){
            char subdir[1024];
            snprintf(subdir, sizeof(subdir), "%s/%s", dirname, entry->d_name);
            processDirectory(subdir, wordFreq, wordFreqSize);
        }
    closedir(dir);
}

void sortByFrequency(struct HashMap *wordFreq, int wordFreqSize) {
    for (int i = 0; i < wordFreqSize - 1; i++) 
        for (int j = i + 1; j < wordFreqSize; j++) 
            if (wordFreq[i].freq < wordFreq[j].freq || (wordFreq[i].freq == wordFreq[j].freq && strcmp(wordFreq[i].word, wordFreq[j].word) > 0)) {
                struct HashMap temp = wordFreq[i];
                wordFreq[i] = wordFreq[j];
                wordFreq[j] = temp;
            }
}

int main(int argc, char *argv[]) {
    if (argc < 2){
        printf("Usage: %s <filename or directory> [filename or directory] ...\n", argv[0]);
        return 1;
    }
    struct HashMap *wordFreq = (struct HashMap *)malloc(INIT_CAPACITY * sizeof(struct HashMap));
    if (wordFreq == NULL){
        perror("malloc");
        return 1; 
    }
    int wordFreqSize = 0;
    for (int i = 1; i < argc; i++){
        struct stat info;
        if (stat(argv[i], &info) != 0){
            perror("stat");
            continue;
        }
        if (S_ISREG(info.st_mode))
            processFile(argv[i], wordFreq, &wordFreqSize);
        else if (S_ISDIR(info.st_mode))
            processDirectory(argv[i], wordFreq, &wordFreqSize);
        else
            printf("%s is neither a file nor a directory.\n", argv[i]);
    }
    sortByFrequency(wordFreq, wordFreqSize);
    for (int i = 0; i < wordFreqSize; i++)
        printf("%s %d\n", wordFreq[i].word, wordFreq[i].freq);
    free(wordFreq);
    return 0;
}