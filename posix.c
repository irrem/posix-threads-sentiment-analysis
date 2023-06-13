#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/syscall.h>

#define MAX_QUEUE_SIZE 100
#define NUM_THREADS 4

typedef struct {
    char* messages[MAX_QUEUE_SIZE];
    int front;
    int rear;
    int count;
    pthread_mutex_t mutex;       // mutex to synchronize access to the queue
    pthread_cond_t full;        // condition variable for indicating the queue is full
    pthread_cond_t empty;       // condition variable for indicating the queue is empty
} Queue;

typedef struct {
    int thread_id;
    Queue* queue;
} ThreadArgs;

int getDataSourceID(int thread_id) {
    return thread_id + 1;
}

char* collectDataFromSource(int data_source_id, int thread_id) {
    /*this is a placeholder code where data is collected from a specific data source and returned as a string
     the relevant data source ID and thread ID will be used
    connect to the Twitter API
    fetch data from the data source
    convert the retrieved data into a string
    return fetched_data;*/
    char* data = (char*)malloc(100 * sizeof(char));
    sprintf(data, "\nData for thread %d", thread_id);
    return data;
}

char* preprocessData(char* data) {
    // preprocessing method
    // data is cleaned and prepared for processing
    return "Preprocessed data";
}

char* performSentimentAnalysis(char* data) {
    // data set is processed using Naive Bayes
    // the resulting sentiment is returned
    return "80% happy";
}

void* dataCollection(void* arg) {
    ThreadArgs* thread_args = (ThreadArgs*) arg;
    int thread_id = thread_args->thread_id;
    Queue* queue = thread_args->queue;
    int data_source_id = getDataSourceID(thread_id);

    char* data = collectDataFromSource(data_source_id, thread_id);

    pthread_mutex_lock(&(queue->mutex));

    while (queue->count == MAX_QUEUE_SIZE) {
        pthread_cond_wait(&(queue->full), &(queue->mutex));
    }
   
    queue->rear = (queue->rear + 1) % MAX_QUEUE_SIZE;
    queue->messages[queue->rear] = data;
    queue->count++;

    pthread_cond_signal(&(queue->empty));
    pthread_mutex_unlock(&(queue->mutex));

    printf("Data collected by thread %d (Thread ID: %lu, Process ID: %d): %s\n", thread_id, (unsigned long)pthread_self(), getpid(), data);

    pthread_exit(data);
}

void* preprocessing(void* arg) {
    ThreadArgs* thread_args = (ThreadArgs*) arg; 
    int thread_id = thread_args->thread_id;
    Queue* queue = thread_args->queue;
    int data_source_id = getDataSourceID(thread_id);
    char* data = (char*)malloc(strlen((char*)arg) + 1);
    strcpy(data, (char*)arg);

    char* preprocessedData = preprocessData(data);

    printf("Test data reached preprocessing: %s\n", preprocessedData);
    printf("Preprocessing thread: %s\n, %d (Thread ID: %lu, Process ID: %d): %s\n", preprocessedData, thread_id, (unsigned long)pthread_self(), getpid(), data);

    free(data);

    pthread_exit(preprocessedData);
}

void* sentimentAnalysis(void* arg) {
    char* data = (char*)malloc(strlen((char*)arg) + 1);
    strcpy(data, (char*)arg);

    char* sentiment = performSentimentAnalysis(data);

    printf("Sentiment analysis result by sentiment analysis thread: %s\n", sentiment);

    free(data);

    pthread_exit(sentiment);
}

int main() {
    Queue queue;
    queue.front = 0;
    queue.rear = -1;
    queue.count = 0;
    pthread_mutex_init(&(queue.mutex), NULL);    // initialize the mutex
    pthread_cond_init(&(queue.full), NULL);      //// initialize the condition variable for full queue
    pthread_cond_init(&(queue.empty), NULL);    // initialize the condition variable for empty queue

    pthread_t threads[NUM_THREADS];
    ThreadArgs thread_args[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; i++) {
        thread_args[i].thread_id = i;
        thread_args[i].queue = &queue;
        pthread_create(&threads[i], NULL, dataCollection, (void*) &thread_args[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        char* data;
        pthread_join(threads[i], (void**) &data);
    }

    pthread_t preprocessingThread, sentimentAnalysisThread;

    pthread_create(&preprocessingThread, NULL, preprocessing, queue.messages[0]);
    pthread_create(&sentimentAnalysisThread, NULL, sentimentAnalysis, queue.messages[0]);

    char* preprocessingResult;
    char* sentimentResult;
    pthread_join(preprocessingThread, (void**) &preprocessingResult);
    //waits for the preprocessingThread to complete and retrieves the output data
    pthread_join(sentimentAnalysisThread, (void**) &sentimentResult);
    //waits for the sentimentAnalysisThread to complete and retrieves the output data

    printf("Preprocessing Result: %s\n", preprocessingResult);
    printf("Sentiment Result: %s\n", sentimentResult);

    pthread_mutex_destroy(&(queue.mutex));
    //destroys the mutex lock
    pthread_cond_destroy(&(queue.full));
    //destroys the full condition variable
    pthread_cond_destroy(&(queue.empty));
    //destroys the empty condition variable

    return 0;
}
