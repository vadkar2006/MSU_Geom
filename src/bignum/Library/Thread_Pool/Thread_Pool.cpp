#pragma onse

#include "Thread_Pool.h"


void Task::Move_To(Task&& task1){
    if(this == &task1)
        return;
    
    std::scoped_lock lock(work, task1.work);
    function = task1.function;
    var = task1.var;
    work_status = task1.work_status;

    task1.function = nullptr;
    task1.var = nullptr;
    task1.work_status = Null_Task;
}





bool Task::Wait_Completed(){
    std::unique_lock<std::mutex> lock(work);
    if(work_status == Null_Task)
        return false;

    if(work_status == Created){
        work_status = Running;
        bool stat = true;
        
        try{
            function(var);
        }
        catch(...){
            stat = false;
        }

        if(stat)
            work_status = Completed;
        else
            work_status = Stop_Error;
    }

    work_var.wait(lock, [this]{return (work_status == Completed) || (work_status == Stop_Error);});
    if(work_status == Completed)
        return true;
    else
        return false;
}
void Task::operator()(){
    std::lock_guard<std::mutex> lock(work);
    work_status = Running;
    bool stat = true;
    
    try{
        function(var);
    }
    catch(...){
        stat = false;
    }

    if(stat)
        work_status = Completed;
    else
        work_status = Stop_Error;
    
    work_var.notify_one();
}





void Thread_Pool::Sending_To_Processing(Task* task) {
    bool flag_push = true;

    {
        std::scoped_lock lock(guard_queue, task -> work);
        if(task -> work_status == Task::Created){
            task_buffer.push(task);
            task -> work_status = Task::Push_In_Pool;
        }
        else{
            flag_push = false;
        }
    }
    
    if(flag_push)
        condition.notify_one();
}

void Thread_Pool::Expand_Thread(int count){
    Flag_Stop = false;

    for(int i = 0; i < count; i++)
        worker.emplace_back(Function_Manager, this);
}
void Thread_Pool::Stop_Thread(){
    {
        std::lock_guard<std::mutex> lock(guard_queue);
        Flag_Stop = true;
    }
    condition.notify_all();

    for(int i = 0; i < worker.size(); i++)
        worker[i].join();
}


