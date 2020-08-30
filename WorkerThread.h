//
// Created by HDTung on 8/30/20.
//

#ifndef EVENTLOOP_WORKERTHREAD_H
#define EVENTLOOP_WORKERTHREAD_H

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <queue>
#include "Fault.h"

#define MSG_EXIT_THREAD			1
#define MSG_POST_USER_DATA		2
#define MSG_TIMER				3

struct UserData{
    std::string msg;
    int year;
};

struct ThreadMsg{
    ThreadMsg(int i, const void* m){id = i; msg = m;}
    int id;
    const void* msg;
};

class WorkerThread{
public:
    //constructor
    WorkerThread(const char* threadName);

    //Destructor
    ~WorkerThread();

    /*
     * Called once to create the worker thread
     * @return TRUE if thread is created. FALSE otherwise.
     */
    bool CreateThread();

    /*
     * Called once a program exit to exit the worker thread
     */
    void ExitThread();

    /*
     * Get the ID of this thread instance
     * @return the worker thread ID
     */
    std::thread::id GetThreadId();

    /*
     * Get the ID of the currently exeecuting thread
     * @return the current thread ID
     */
    static std::thread::id GetCurrentThreadId();

    /*
     * Add a message to thread queue
     * @param[in] data - thread specific information created on the heap using
     * operator new.
     */
    void PostMsg(const UserData* data);

    /*
     * Move to thread
     */
    //void MoveToThread(std::thread& thread);

private:
    WorkerThread(const WorkerThread&);
    WorkerThread& operator = (const WorkerThread&);

    /*
     * Entry point for the worker thread
     */
    void Process();

    /*
     * Entry point for timer thread
     */
    void TimerThread();

    std::thread* m_thread;
    std::queue<ThreadMsg*> m_queue;
    std::mutex m_mutex;
    std::condition_variable m_cv;
    std::atomic<bool> m_timerExit;
    const char* THREAD_NAME;
};

WorkerThread::WorkerThread(const char* threadName) : m_thread(nullptr), m_timerExit(false), THREAD_NAME(threadName)
{
}

WorkerThread::~WorkerThread()
{
    ExitThread();
}

bool WorkerThread::CreateThread() {
    if(!m_thread)
        m_thread = new std::thread(&WorkerThread::Process, this);
    return true;
}

std::thread::id WorkerThread::GetThreadId() {
    ASSERT_TRUE(m_thread != 0);
    return m_thread->get_id();
}

std::thread::id WorkerThread::GetCurrentThreadId() {
    return std::this_thread::get_id();
}

void WorkerThread::ExitThread() {
    if(!m_thread)
        return;

    //Create a new ThreadMsg
    ThreadMsg* threadMsg = new ThreadMsg(MSG_EXIT_THREAD, 0);

    //Put exit thread message into the queue
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push(threadMsg);
        m_cv.notify_one();
    }
    m_thread->join();
    delete m_thread;
    m_thread = nullptr;
}

void WorkerThread::PostMsg(const UserData* data) {
    ASSERT_TRUE(m_thread);

    ThreadMsg* threadMsg = new ThreadMsg(MSG_POST_USER_DATA, data);

    //Add user data msg to queue and notify worker thread
    std::unique_lock<std::mutex> lk(m_mutex);
    m_queue.push(threadMsg);
    m_cv.notify_one();
}

void WorkerThread::TimerThread() {
    while(!m_timerExit)
    {
        //Sleep for 250ms then put a MSG_TIMER message into queue
        std::this_thread::sleep_for(std::chrono::milliseconds(250));

        ThreadMsg* threadMsg = new ThreadMsg(MSG_TIMER, 0);

        //Add timer msg to queue and notify worker thread
        std::unique_lock<std::mutex> lk(m_mutex);
        m_queue.push(threadMsg);
        m_cv.notify_one();
    }
}

void  WorkerThread::Process() {
    m_timerExit = false;
    std::thread timerThread(&WorkerThread::TimerThread, this);

    while (true)
    {
        ThreadMsg* threadMsg = nullptr;
        {
            //Wait for a message to be added to the queue
            std::unique_lock<std::mutex> lk(m_mutex);
            while (m_queue.empty())
                m_cv.wait(lk,[&]{ return !m_queue.empty();});
            threadMsg = m_queue.front();
            m_queue.pop();
        }

        switch (threadMsg->id) {
            case MSG_POST_USER_DATA:
            {
                ASSERT_TRUE(threadMsg->msg != NULL);

                //convert the ThreadMsg void* data back to a UserData*
                const UserData* userData = static_cast<const UserData*>(threadMsg->msg);
                std::cout << userData->msg.c_str() << " " << userData->year << " on " << THREAD_NAME << std::endl;

                //Delete dynamic data passed through message queue
                delete userData;
                delete threadMsg;
                break;
            }

            case MSG_TIMER:
                std::cout << "Timer expired on " << THREAD_NAME << std::endl;
                delete threadMsg;
                break;

            case MSG_EXIT_THREAD:
            {
                m_timerExit = true;
                timerThread.join();
                delete threadMsg;

                //clear all message on queue
                std::unique_lock<std::mutex> lk(m_mutex);
                while (!m_queue.empty())
                {
                    threadMsg = m_queue.front();
                    m_queue.pop();
                    delete threadMsg;
                }
                std::cout << "Exit thread on " << THREAD_NAME << std::endl;
                return;
            }
            default:
                ASSERT();
        }
    }
}

#endif //EVENTLOOP_WORKERTHREAD_H
