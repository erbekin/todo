#include "todo.h"


// ** UTILS **//

void formatTime(time_t rawTime, char *buffer) {
    struct tm *timeInfo;


    timeInfo = localtime(&rawTime);

    strftime(buffer, 100, "%d %B %Y %H:%M:%S", timeInfo);
}

long getFileSize(FILE *file) {
    
    if (file == NULL) {
        return -1;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fclose(file);

    return size;
}

int fileExists(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (file) {
        fclose(file); //
        return 1;
    }
    return 0;
}

bool computeTodoHash(TodoFile tf, TodoHash *hash) {
    if (!hash) return false;  // Check hash pointer upfront
    
    unsigned long fileSize = sizeof(Task) * TODO_MAX_TODO;
    TodoHash hashHash;
    
    // init hasher
    EVP_MD_CTX *mdctx;
    if((mdctx = EVP_MD_CTX_new()) == NULL) {
        return false;
    }
    if(1 != EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL)) {
        EVP_MD_CTX_free(mdctx);
        return false;
    }
    
    // buffer for file contents
    unsigned char buffer[fileSize];
    seekToTask(tf, 0);

    // read all tasks
    size_t bytesRead = fread(buffer, 1, fileSize, tf.file);
    if (bytesRead != fileSize) {
        EVP_MD_CTX_free(mdctx);
        return false;
    }

    // hash file contents
    if(1 != EVP_DigestUpdate(mdctx, buffer, fileSize)) {
        EVP_MD_CTX_free(mdctx);
        return false;
    }
    
    unsigned int length = 0;
    if(1 != EVP_DigestFinal_ex(mdctx, hashHash.hash, &length)) {
        EVP_MD_CTX_free(mdctx);
        return false;
    }
    EVP_MD_CTX_free(mdctx);

    // xor result hash with secret key
    const char* key = TODO_SECRET_KEY;
    size_t keylen = strlen(key);
    
    // Ensure we have a key with some length
    if (keylen == 0) {
        return false;
    }
    
    for (int i = 0; i < TODO_FILE_HASH_SIZE; i++) {
        hashHash.hash[i] = hashHash.hash[i] ^ key[i % keylen];
    }
    
    memcpy(hash, &hashHash, sizeof(TodoHash));
    return true;  // Return true on success
}

TodoFile openTodo() {
    TodoFile tf;
    FILE *f = NULL;
    if (!fileExists(TODO_FILE_NAME)) {
        f = fopen(TODO_FILE_NAME, "wb+");
        if (!f) {
            tf.status = FILE_NOT_FOUND;
            return tf;
        }
        DEBUG_LOG("New file will be formatted");
        formatFile(f);
        tf.mdata = getFileMetadata(f);
        tf.file = f;
        tf.status = FILE_IS_READY;
        DEBUG_LOG("Opened new todo");
        return tf;
    }
    else {
        f = fopen(TODO_FILE_NAME, "rb+");
        if (!f) {
            tf.status = FILE_NOT_FOUND;
            return tf;
        }
        tf.mdata = getFileMetadata(f);
        tf.file = f;
        tf.status = FILE_IS_READY;
        
        TodoHash prev;
        if (!computeTodoHash(tf, &prev)) {
            DEBUG_LOG("Couldn't compute hash\n");
            fclose(tf.file);
            tf.status = FILE_NOT_FOUND;
            return tf;
        }
        if (memcmp(prev.hash, tf.mdata.hash.hash, TODO_FILE_HASH_SIZE) != 0) {
            DEBUG_LOG("Hash was not matched\n");
            tf.status = FILE_WAS_CHANGED;
            fclose(tf.file);
            return tf;
        }
        DEBUG_LOG("Hash matched, existing todo opened");
        return tf;   
    }
}

MetaData getFileMetadata(FILE *f) {
    MetaData md;
    fseek(f, 0, SEEK_SET);
    fread(&md, sizeof(md), 1, f);
    return md;
}

void formatFile(FILE *f) {
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
    MetaData meta = {
        .capacity = TODO_MAX_TODO,
        .activeCount = 0,
        .completedCount = 0,
    };
    fwrite(&meta, sizeof(meta), 1, f);

    // insert tasks
    unsigned long cap = TODO_MAX_TODO;
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
        return;
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

    TodoHash newHash;
    if (!computeTodoHash(tf, &newHash)) {
        DEBUG_LOG("Couldn't compute hash while updating metadata\n");
        return;
    }
    newMeta.hash = newHash;

    fseek(tf.file, 0, SEEK_SET);
    fwrite(&newMeta, sizeof(MetaData), 1, tf.file);
    DEBUG_LOG("Metadata updated and wrote");
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

ID addTask(TodoFile tf, const char *description) {
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
    DEBUG_LOG("Added new task to %lu", taskCounter.id);
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



