#include <stdio.h>
#include <time.h>
#include "escheduler.h"

class Task1 : public ETask
{
public:
    Task1() : m_wakeup(false)
    { }

    void wakeup()
    {
        m_wakeup = true;
    }

    virtual void run()
    {
    E_BEGIN_TASK

        printf("Task 1 - step 1\n");
        E_REENTER_DELAY(2000);

        printf("Task 1 - step 2\n");
        E_REENTER_COND(m_wakeup);

        printf("Task 1 - step 3\n");

        while(true) {
            printf("Task 1 - in loop\n");
            E_REENTER_DELAY(1500);
        }

    E_END_TASK
    }

private:
    bool m_wakeup;
};

class Task2 : public ETask
{
public:
    Task2(Task1& task1) : m_task1(task1)
    { }

    virtual void run()
    {
    E_BEGIN_TASK

        printf("Task 2 - step 1\n");
        E_REENTER_DELAY(3000);

        printf("Task 2 - step 2\n");
        E_REENTER_DELAY(2000);

        printf("Waking up task 1\n");
        m_task1.wakeup();
        E_REENTER_DELAY(3000);

        while(true) {
            printf("Task 2 - in loop\n");
            E_REENTER_DELAY(1000);
        }

    E_END_TASK
    }

private:
    Task1& m_task1;
};

uint64_t tickFunction()
{
    uint64_t ms;
    struct timespec spec;

    clock_gettime(CLOCK_REALTIME, &spec);
    
    ms  = spec.tv_sec * 1000;
    ms += spec.tv_nsec / 1.0e6;

    return ms;
}

int main(int argc, char * argv[])
{
    Task1 task1;
    Task2 task2(task1);

    ETask* taskList[] = {&task1, &task2, NULL};

    EScheduler s(&tickFunction, taskList);
    s.start();
}
