#pragma once

#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>


class Task;
class Thread_Pool;


class Task{
    enum Work_Status{Null_Task, Created, Push_In_Pool, Running, Completed, Stop_Error};
    void (*function)(void** var);
    void** var;

    Work_Status work_status;

    mutable std::mutex work;
    std::condition_variable work_var;

public:
    Task(){
        function = nullptr;
        var = nullptr;
        work_status = Null_Task;
    }
    Task(void (*func)(void** var), void** var_new){
        function = func;
        var = var_new;
        work_status = Created;
    }
    Task(const Task& task1) = delete;
    
    void Move_To(Task&& task1);


    bool Wait_Completed();
    void operator()();

    ~Task(){Wait_Completed();}

    friend Thread_Pool;
};




class Thread_Pool{
    std::vector<std::thread> worker;
    std::queue<Task*> task_buffer;

    std::condition_variable condition;
    bool Flag_Stop;
    
    mutable std::mutex guard_queue;



    static void Function_Manager(Thread_Pool* this_pool) {
        std::unique_lock<std::mutex> lock1(this_pool -> guard_queue);

        while(true) {
            this_pool -> condition.wait(lock1, [this_pool]() {
                return !(this_pool -> task_buffer.empty()) || (this_pool -> Flag_Stop);
            });
            
            if((this_pool -> Flag_Stop) && (this_pool -> task_buffer.empty()))
                return;


            
            Task* task = this_pool -> task_buffer.front();
            this_pool -> task_buffer.pop();
            lock1.unlock();
            
            (*task)();

            lock1.lock();
        }
    }

    void Expand_Thread(int count = std::thread::hardware_concurrency());
    void Stop_Thread();
public:

    Thread_Pool(){Expand_Thread();}
    Thread_Pool(int n){Expand_Thread(n);}

    void Sending_To_Processing(Task* task);

    int Count_Worker(){return worker.size();}


    ~Thread_Pool(){Stop_Thread();}
};



