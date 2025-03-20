#ifndef TODO_HEADER_
#define TODO_HEADER_
#include <time.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <openssl/evp.h>

#pragma region todo_types

#define TODO_DESCRIPTION_SIZE 256
#define TODO_FILE_NAME ".todo"
#define TODO_MAX_TODO 1000
#define TODO_FILE_HASH_SIZE 32
#define TODO_SECRET_KEY "a1b2c3d4e5f6g7h8i9j0k1l2m3n4o5p6"
typedef unsigned long ID;

#ifdef DEBUG
#define DEBUG_LOG(fmt, ...) fprintf(stderr, "[DEBUG] " fmt "\n", ##__VA_ARGS__)
#else
#define DEBUG_LOG(fmt, ...) /* do nothing */
#endif

/**
 * @struct Task
 * @brief Represents a task in the todo list.
 * 
 * @var Task::id
 * Task ID, unique identifier for the task.
 * 
 * @var Task::active
 * Indicates if the task is active. If not active, other values are meaningless.
 * 
 * @var Task::description
 * Description of the task.
 * 
 * @var Task::datetime
 * Date and time when the task was created.
 * 
 * @var Task::completed
 * Indicates if the task is completed.
 */
typedef struct Task {
    ID id;
    bool active;
    char description[TODO_DESCRIPTION_SIZE];
    time_t datetime;
    bool completed;
} Task;


typedef struct TodoHash {
    unsigned char hash[TODO_FILE_HASH_SIZE];
} TodoHash;


/**
 * @struct MetaData
 * @brief Metadata information for the todo file.
 * 
 * @var MetaData::capacity 
 * Total number of tasks that can be stored in the file.
 * 
 * @var MetaData::completedCount
 * Number of tasks that have been completed.
 * 
 * @var MetaData::activeCount
 * Number of active tasks.
 */
typedef struct MetaData {
    TodoHash hash;
    unsigned long capacity;
    unsigned long completedCount;
    unsigned long activeCount;
} MetaData;



/**
 * @enum StatusCode
 * @brief Status codes for file operations.
 * 
 * @var StatusCode::FILE_NOT_FOUND
 * Indicates that the file was not found or there was an error opening it.
 * 
 * @var StatusCode::FILE_IS_READY
 * Indicates that the file was successfully opened or formatted for use.
 */
enum StatusCode {
    FILE_NOT_FOUND,
    FILE_IS_READY,
    FILE_WAS_CHANGED
};

/**
 * @struct TodoFile
 * @brief Represents a todo file with its metadata and status.
 * 
 * @var TodoFile::file
 * File pointer to the todo file.
 * 
 * @var TodoFile::status
 * Status of the file (e.g., ready, not found).
 * 
 * @var TodoFile::mdata
 * Metadata information of the todo file.
 */
typedef struct TodoFile {
    FILE *file;
    enum StatusCode status;
    MetaData mdata;
} TodoFile;

#pragma endregion

#pragma region todo_file

/**
 * @brief Opens the todo file if it exists, otherwise creates and formats a new file.
 * 
 * @return TodoFile Struct representing the opened file.
 */
TodoFile openTodo();

/**
 * @brief Closes the given todo file.
 * 
 * @param tf The TodoFile struct representing the file to close.
 */
void closeFile(TodoFile tf);

/**
 * @brief Formats the given file with the provided metadata.
 * 
 * @param f The file pointer to format.
 */
void formatFile(FILE *f);

/**
 * @brief Retrieves the metadata from the beginning of the file.
 * Assumes the file is valid.
 * 
 * @param f The file pointer to read metadata from.
 * @return MetaData The metadata read from the file.
 */
MetaData getFileMetadata(FILE *f);

/**
 * @brief Updates the metadata information by checking all tasks in the file.
 * This operation can be intensive.
 * 
 * @param tf The TodoFile struct representing the file to update.
 */
void updateMetadata(TodoFile tf);

#pragma endregion

#pragma region todo_task

/**
 * @brief Checks if a task with the given ID exists in the file.
 * 
 * @param tf The TodoFile struct representing the file to check.
 * @param id The ID of the task to check.
 * @return true If the task ID exists in the file.
 * @return false If the task ID does not exist in the file.
 */
bool taskIdExists(TodoFile tf, ID id);

/**
 * @brief Checks if the given task exists in the file.
 * This is equivalent to checking by ID but accepts a Task object.
 * 
 * @param tf The TodoFile struct representing the file to check.
 * @param t The Task object to check.
 * @return true If the task exists in the file.
 * @return false If the task does not exist in the file.
 */
bool taskExists(TodoFile tf, Task t);

/**
 * @brief Checks if the task with the given ID is active.
 * 
 * @param tf The TodoFile struct representing the file to check.
 * @param id The ID of the task to check.
 * @return true If the task is active.
 * @return false If the task is not active or the ID is not valid.
 */
bool taskActive(TodoFile tf, ID id);

/**
 * @brief Seeks the file to the position of the task with the given ID.
 * The ID must be valid.
 * 
 * @param tf The TodoFile struct representing the file to seek.
 * @param task The ID of the task to seek to.
 */
void seekToTask(TodoFile tf, ID task);

/**
 * @brief Retrieves the task with the given ID from the file.
 * If the file is not ready, returns a task with an invalid ID.
 * 
 * @param tf The TodoFile struct representing the file to read from.
 * @param id The ID of the task to retrieve.
 * @return Task The task read from the file.
 */
Task getTaskById(TodoFile tf, ID id);

/**
 * @brief Adds a new task to the file with the given description.
 * If there is no room for a new task, the task will not be added.
 * 
 * @param tf The TodoFile struct representing the file to add to.
 * @param description The description of the new task.
 * @return ID The ID of the newly added task. If the ID equals the capacity, there was an error.
 */
ID addTask(TodoFile tf, const char *description);

#pragma endregion

/***************** UTILS **********************************/
#pragma region utils

// Compute hash of todo file with xoring secret key
bool computeTodoHash(TodoFile tf, TodoHash *hash);

/**
 * @brief Returns the size of the given file.
 * 
 * @param file The file pointer to check.
 * @return long The size of the file in bytes.
 */
long getFileSize(FILE *file);

/**
 * @brief Checks if a file with the given name exists and is openable for reading.
 * 
 * @param filename The name of the file to check.
 * @return int 1 if the file exists, 0 otherwise.
 */
int fileExists(const char *filename);

/**
 * @brief Prints the given task to stdout.
 * 
 * @param t The task to print.
 */
void printTask(Task t);

/**
 * @brief Formats the given time into a human-readable string.
 * 
 * @param rawTime The raw time to format.
 * @param buffer The buffer to store the formatted time string.
 */
void formatTime(time_t rawTime, char *buffer);

#pragma endregion
#endif // TODO_HEADER_