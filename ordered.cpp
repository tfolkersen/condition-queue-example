/*
 *			ORDERED FILE
 */
#include <iostream>
#include <pthread.h>
#include <stdlib.h>

#define ITERS 1000
#define N 400
#define TOTAL ITERS * N
#define WASTEITERATIONS 0
//#define PRINT
//#define THREADPRINT

using namespace std;

struct queueItem {
public:
	pthread_cond_t cond;
	queueItem *next;
	int id;
};

queueItem *front = NULL;
queueItem *back = NULL;

queueItem *qi_new(int id) {
	queueItem *item = (queueItem *) malloc(sizeof(queueItem));
	item->cond = PTHREAD_COND_INITIALIZER;
	item->next = NULL;
	item->id = id;
	return item;
}

void qi_delete(queueItem *item) {
	free(item); //TODO Might leak condition resources I don't know lmao
}

//CALL WHILE HOLDING THE MUTEX !!!IMPORTANT!!!
void enterQueue(pthread_mutex_t *mut, int id) { //Pass the mutex that protects the condition queue
	queueItem *item = qi_new(id);
	if (back == NULL) {
		back = item;
		front = item;
	} else {
		back->next = item;
		back = item;
	}
	pthread_cond_wait(&item->cond, mut);
}

void exitQueue(int id) {
	if (front->id != id) {
		cout << "Queue IDs don't match somehow..." << endl;
	}

	queueItem *item = front;

	if (item->next != NULL) {
		front = item->next;
	} else {
		front = NULL;
		back = NULL;
	}
	qi_delete(item);
}

void signalQueue() {
	if (front == NULL) {
		cout << "Tried to signal empty queue" << endl;
		return;
	}
	pthread_cond_signal(&front->cond);
}

int count = 0;
pthread_mutex_t condMut = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

int requestOrder[TOTAL];
int actOrder[TOTAL];
int requestIndex;
int actIndex;

int busy = 0;
int waiting = 0;
int maxWaiting = 0;

int wasteTime(int val) {
	int acc = 0;
	for (int i = 0; i < WASTEITERATIONS; i++) {
		acc += val + acc;
	}
	return acc;
}

void *threadTask(void *arg) {
	int id = *((int *) arg);
	#if defined(THREADPRINT)
	cout << "Starting thread " << id << endl;
	#endif


	for (int i = 0; i < ITERS; i++) {
		while (pthread_mutex_lock(&condMut)) {} //Acquire condMut
		requestOrder[requestIndex++] = id; //REQUEST
		if (busy) { //Wait on condition
			waiting++;
			maxWaiting = max(maxWaiting, waiting);
			enterQueue(&condMut, id);
			exitQueue(id); //TODO make into one function
			waiting--;
		}
		busy = 1;
		actOrder[actIndex++] = id; //ACT
		pthread_mutex_unlock(&condMut);

		count++;
		wasteTime(id);
		while (pthread_mutex_lock(&condMut)) {} //Acquire condMut
		if (waiting > 0) {
			signalQueue();
			//pthread_cond_signal(&cond);
		} else {
			busy = 0;
		}
		pthread_mutex_unlock(&condMut); //This line allows next thread to continue, (see unordered.cpp)

	}


	return arg;
}

int main() {
	cout << "Ordered file" << endl;
	pthread_t threads[N];
	int ids[N];
	for (int i = 0; i < N; i++) {
		ids[i] = i + 1;
	}

	for (int i = 0; i < N; i++) {
		pthread_create(threads + i, 0, threadTask, ids + i);
	}
	
	for (int i = 0; i < N; i++) {
		void *ret;
		pthread_join(threads[i], &ret);
		#if defined(THREADPRINT)
		int id = *((int *) ret);
		cout << "Returning " << id << endl;
		#endif
	}

	cout << "Expected " <<  TOTAL << endl;
	cout << "Actual " << count << endl;
	cout << "Waiting " << waiting << endl;
	cout << "Max waiting " << maxWaiting << endl;
	cout << "Request index " << requestIndex << endl;
	cout << "Act index " << actIndex << endl;

	cout << "R: ";
	for (int i = 0; i < requestIndex; i++) {
		#if defined(PRINT)
		cout << requestOrder[i];
		#endif
	}
	cout << endl;
	cout << "A: ";
	for (int i = 0; i < actIndex; i++) {
		#if defined(PRINT)
		cout << actOrder[i];
		#endif
	}
	cout << endl;

	cout << "Differences: ";
	if (actIndex != requestIndex) {
		cout << "Indices are different somehow..." << endl;
	} else {
		for (int i = 0; i < actIndex; i++) {
			if (requestOrder[i] != actOrder[i]) {
				cout << i << " ";
			}
		}
	}
	cout << endl;

	cout << "Done!" << endl;
	return 0;
}
