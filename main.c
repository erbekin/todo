#include "todo.h"

int main(void) {
    TodoFile tf = openFile();
    if (tf.status != FILE_IS_READY) {
        printf("error while opening\n");
        return 1;
    }
    printf("Cap: %lu\n", tf.mdata.capacity);
    printf("Active: %lu\n", tf.mdata.activeCount);
    printf("Completed: %lu\n", tf.mdata.completedCount);
    printTask(getTaskById(tf, 0));
    closeFile(tf);
}