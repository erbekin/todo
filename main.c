#include "todo.h"

int main(void) {
    const TodoFile tf = openTodo();
    if (tf.status != FILE_IS_READY) {
        printf("error while opening\n");
        return 1;
    }
    addTask(tf, "Ekmek al");
    closeFile(tf);
}