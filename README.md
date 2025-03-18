# Todo List Application

This is a simple Todo List application written in C. It allows you to manage tasks with descriptions, creation dates, and completion statuses.

## Project Structure

## Files

- `main.c`: Contains the `main` function which initializes and interacts with the Todo list.
- `todo.c`: Contains the implementation of the Todo list functions.
- `todo.h`: Contains the declarations and data structures used in the Todo list application.


## Usage

1. **Open the Project**: Open the project in Visual Studio Code.
2. **Build the Project**: Use the build command to compile the project.
3. **Run the Application**: Execute the compiled binary to run the Todo list application.

## Functions

### File Operations

- `TodoFile openFile()`: Opens the Todo file if it exists, otherwise creates and formats a new file.
- `void closeFile(TodoFile tf)`: Closes the Todo file and updates metadata.
- `MetaData getFileMetadata(FILE *f)`: Retrieves metadata from the Todo file.
- `void formatFile(FILE *f, MetaData meta)`: Formats the Todo file with initial metadata and tasks.
- `void updateMetadata(TodoFile tf)`: Updates the metadata by checking all tasks.

### Task Operations

- `bool taskIdExists(TodoFile tf, ID id)`: Checks if a task ID exists in the file.
- `bool taskExists(TodoFile tf, Task t)`: Checks if a task exists in the file.
- `bool taskActive(TodoFile tf, ID id)`: Checks if a task is active.
- `Task getTaskById(TodoFile tf, ID id)`: Retrieves a task by its ID.
- `ID AddTask(TodoFile tf, const char *description)`: Adds a new task with the given description.
- `void printTask(Task t)`: Prints the details of a task.

### Utility Functions

- `long getFileSize(FILE *file)`: Returns the size of the file.
- `int fileExists(const char *filename)`: Checks if a file exists.
- `void formatTime(time_t rawTime, char *buffer)`: Formats the time into a readable string.

## License

This project is licensed under the MIT License.