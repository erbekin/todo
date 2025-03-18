#ifndef TODO_HEADER_
#define TODO_HEADER_
#include <time.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#pragma region todo_types

#define TODO_DESCRIPTION_SIZE 256
#define TODO_FILE_NAME ".todo"
#define TODO_MAX_TODO 1000

typedef unsigned long ID;

typedef struct Task {
    ID id;
    bool active; // if not active the task cell is means empty, and other values other than id is meaningless
    char description[TODO_DESCRIPTION_SIZE];
    time_t datetime;  // Görev için tam tarih ve saat
    bool completed;
} Task;





typedef struct MetaData {
    unsigned long capacity; // how many task in file
    unsigned long completedCount; // how many task completed
    unsigned long activeCount; // how many task is active
} MetaData;

enum StatusCode {
    FILE_NOT_FOUND, // there was an error while opening or finding
    FILE_IS_READY // file opened or formatted for using
};

typedef struct TodoFile {
    FILE *file;
    enum StatusCode status;
    MetaData mdata;
} TodoFile;

#pragma endregion

#pragma region todo_file
// Opens file if exists otherwise creates and formats new file. Returns TodoFile struct of opened file.
TodoFile openFile();

void closeFile(TodoFile tf);

void formatFile(FILE *f, MetaData meta);

// get file metadata from beginning,
// assumes file is valid
MetaData getFileMetadata(FILE *f);

// update metadata info checking all todos,
// can be intensive
TodoFile updateMetadata(TodoFile tf);


#pragma endregion


#pragma region todo_task

// returns true if task id exists in file
// otherwise false
bool taskIdExists(TodoFile tf, ID id);


// returns true if task exists in file
// this is equivalent of calling by id
// the only difference is accepts Task object
bool taskExists(TodoFile tf, Task t);

// returns true if the task is active
// when id is not valid returns false
bool taskActive(TodoFile tf, ID id);

// Seeks file to given task id.
// id must be valid
void seekToTask(TodoFile tf, ID task);


// return task struct in the file,
// if file is not ready, returns task with id of capacity value which is invalid
Task getTaskById(TodoFile tf, ID id);

// Add new task to file by giving description
// if file does not have any room for new task
// task wont be added. you can check return value for this
// returns id of new added task, if id is equals capacity
// there was an error probably.
ID AddTask(TodoFile tf, const char *description);


/***************** UTILS **********************************/
#pragma region utils

// returns size of the file
long getFileSize(FILE *file);

// returns bool true if exists (openable for reading) false if not
int fileExists(const char *filename);

// print a task into stdout
void printTask(Task t);

void formatTime(time_t rawTime, char *buffer);

#pragma endregion
#endif // TODO_HEADER_