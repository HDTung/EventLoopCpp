#include <iostream>
#include "WorkerThread.h"

// Worker thread instances
WorkerThread workerThread1("WorkerThread1");
WorkerThread workerThread2("WorkerThread2");

void print()
{
    std::cout << "Test \n";
}

int main() {

    // Create worker threads
    workerThread1.CreateThread();
    workerThread2.CreateThread();

    // Create message to send to worker thread 1
    UserData* userData1 = new UserData();
    userData1->msg = "Hello World";
    userData1->year = 2020;

    // Post the message to worker thread 1
    workerThread1.PostMsg(userData1);

    // Create message to send to work thread 2
    UserData* userData2 = new UserData();
    userData2->msg = "Goodbye World";
    userData2->year = 2020;

    // Post the message to work thread 2
    workerThread2.PostMsg(userData2);

    // Give time for messages processing on worker threads
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    workerThread1.ExitThread();
    workerThread2.ExitThread();

    return 0;
}
