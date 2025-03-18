#include "todo.h"

// ** UTILS **//

void formatTime(time_t rawTime, char *buffer) {
    struct tm *timeInfo;

    // time_t'ı yerel zamana çevir
    timeInfo = localtime(&rawTime);

    // Formatlı tarihi oluştur ve buffer'a yaz
    strftime(buffer, 100, "%d %B %Y %H:%M:%S", timeInfo);
}

long getFileSize(FILE *file) {
    
    if (file == NULL) {
        return -1;
    }

    fseek(file, 0, SEEK_END);  // Dosyanın sonuna git
    long size = ftell(file);   // Mevcut konum (dosya boyutu)
    fclose(file);              // Dosyayı kapat

    return size;
}

int fileExists(const char *filename) {
    FILE *file = fopen(filename, "rb"); // Dosyayı sadece okuma modunda açmayı dene
    if (file) {
        fclose(file); // Açıldıysa kapat
        return 1;     // Dosya var
    }
    return 0;         // Dosya yok
}


TodoFile openFile() {
    TodoFile tf;
    FILE *f = NULL;
    if (!fileExists(TODO_FILE_NAME)) {
        f = fopen(TODO_FILE_NAME, "wb+");
        if (!f) {
            tf.status = FILE_NOT_FOUND;
            return tf;
        }
        MetaData m;
        memset(&m, 0, sizeof(m));
        m.capacity = TODO_MAX_TODO;
        formatFile(f, m);
    }
    else {
        f = fopen(TODO_FILE_NAME, "rb+");
        if (!f) {
            tf.status = FILE_NOT_FOUND;
            return tf;
        }
    }
    tf.mdata = getFileMetadata(f);
    tf.file = f;
    tf.status = FILE_IS_READY;
    return tf;
}

MetaData getFileMetadata(FILE *f) {
    MetaData md;
    fseek(f, 0, SEEK_SET);
    fread(&md, sizeof(md), 1, f);
    return md;
}

void formatFile(FILE *f, MetaData meta) {
    time_t now;
    time(&now);
    unsigned long nextId = 0;
    Task sample = {
        .id = nextId,
        .completed = false,
        .active = false,
        .datetime = now,
        .description = "",
    };
    // start from beginning
    fseek(f, 0, SEEK_SET);
    // insert metadata
    fwrite(&meta, sizeof(meta), 1, f);

    // insert tasks
    unsigned long cap = meta.capacity;
    for (unsigned long i = 0; i < cap; i++) {
        sample.id = nextId++;
        fwrite(&sample, sizeof(sample), 1, f);
    }
   
}

void closeFile(TodoFile tf) {
    if (tf.status == FILE_IS_READY) {
        updateMetadata(tf);
        fclose(tf.file);
    }

}

bool taskIdExists(TodoFile tf, ID id) {
    unsigned long cap = tf.mdata.capacity;
    if (id < cap) return true;
    return false;
}


bool taskExists(TodoFile tf, Task t) {
    return taskIdExists(tf, t.id);
}

bool taskActive(TodoFile tf, ID id) {
    if (tf.status != FILE_IS_READY || !taskIdExists(tf, id)) return false;
    Task inId = getTaskById(tf, id);
    if (inId.active) return true;
    return false;
}


void updateMetadata(TodoFile tf) {
    if (tf.status != FILE_IS_READY) {
        return tf;
    }

    MetaData oldMeta = tf.mdata;
    MetaData newMeta = {
        .capacity = oldMeta.capacity, // not change
        .completedCount = 0,
        .activeCount = 0,
    };

    const unsigned long capacity = oldMeta.capacity;

    // skip into tasks
    fseek(tf.file, sizeof(MetaData), SEEK_SET);

    for (unsigned long i = 0; i < capacity; i++) {
        Task curr;
        fread(&curr, sizeof(curr), 1, tf.file);
        
        // check

        if (!curr.active) {
            continue;
        }
        else if (curr.active) {
            newMeta.activeCount++;
        }
        else if (curr.completed) {
            newMeta.completedCount++;    
        }    
    }
    fseek(tf.file, 0, SEEK_SET);
    fwrite(&newMeta, sizeof(MetaData), 1, tf.file);
}

void seekToTask(TodoFile tf, ID id) {
    if (tf.status != FILE_IS_READY) return;
    if (!taskIdExists(tf, id)) return;
    fseek(tf.file, (long)(sizeof(tf.mdata) + (id * sizeof(Task))), SEEK_SET);
}

Task getTaskById(TodoFile tf, ID id) {
    Task rTask;
    rTask.id = tf.mdata.capacity;
    if (tf.status != FILE_IS_READY) return rTask;
    if (!taskIdExists(tf, id)) return rTask;
    seekToTask(tf, id);
    fread(&rTask, sizeof(Task), 1, tf.file);
    return rTask;
}

ID AddTask(TodoFile tf, const char *description) {
    if (tf.status != FILE_IS_READY) return tf.mdata.capacity;
    seekToTask(tf, 0);
    ID idCounter = 0;
    Task taskCounter;
    do {
        taskCounter = getTaskById(tf, idCounter++);
    } while (taskIdExists(tf, idCounter) && taskCounter.active);
    if (taskCounter.active) return tf.mdata.capacity;
    strncpy(taskCounter.description, description, TODO_DESCRIPTION_SIZE);
    taskCounter.description[TODO_DESCRIPTION_SIZE - 1] = '\0';

    time_t now;
    time(&now);
    taskCounter.datetime = now;
    taskCounter.active = true;
    taskCounter.completed = false;

    seekToTask(tf, taskCounter.id);
    fwrite(&taskCounter, sizeof(Task), 1, tf.file);
    return taskCounter.id;
}

void printTask(Task t) {
    char fmtDate[50];
    formatTime(t.datetime, fmtDate);
    printf("Task<%lu>:\n", t.id);
    printf("Description:\n\t%s\n", t.description);
    printf("Created at: %s\n", fmtDate);
    printf("Completed: %d\n", t.completed);
    printf("Active: %d\n", t.active);    
}



