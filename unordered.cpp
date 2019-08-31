/*
 *		UNORDERED FILE
 *
 */
#include <iostream>
#include <pthread.h>

#define ITERS 10000
#define N 40
#define TOTAL ITERS * N
#define WASTEITERATIONS 0
//#define PRINT
//#define THREADPRINT

using namespace std;

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
			pthread_cond_wait(&cond, &condMut);
			waiting--;
		}
		busy = 1;
		actOrder[actIndex++] = id; //ACT
		pthread_mutex_unlock(&condMut);

		count++;
		wasteTime(id);
		while (pthread_mutex_lock(&condMut)) {} //Acquire condMut
		if (waiting > 0) {
			pthread_cond_signal(&cond);
		} else {
			busy = 0;
		}
		pthread_mutex_unlock(&condMut); //This line allows the signalled thread to continue -- removing it causes the program to hang

	}


	return arg;
}

int main() {
	cout << "Unordered file" << endl;
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
