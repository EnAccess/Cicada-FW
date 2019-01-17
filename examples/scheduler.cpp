#include <stdio.h>
#include <time.h>
#include "escheduler.h"

class Task1 : public ETask
{
public:
    Task1() : ETask(this), m_wakeup(false)
    { }

    void wakeup()
    {
        m_wakeup = true;
    }

    void start()
    {
        printf("Task 1 - step 1\n");
        next(&Task1::stepTwo, 3000);
    }

    void stepTwo()
    {
        printf("Task 1 - step 2\n");
        next(&Task1::stepThree, m_wakeup);
    }

    void stepThree()
    {
        printf("Task 1 - in loop\n");
        next(&Task1::stepThree, 1000);
    }

private:
    bool m_wakeup;
};

class Task2 : public ETask
{
public:
    Task2(Task1& task1) : ETask(this), m_task1(task1)
    { }

    void start ()
    {
        printf("Task 2 - step 1\n");
        next(&Task2::stepTwo, 5000);
    }

    void stepTwo ()
    {
        printf("Task 2 - step 2\n");
        printf("Waking up task 1\n");
        m_task1.wakeup();

        next(&Task2::stepThree, 10000);
    }

    void stepThree ()
    {
        printf("Task 2 - step 3\n");
        next(&Task2::stepFour, 1000);
    }

    void stepFour ()
    {
        printf("Task 2 - in loop\n");
        next(&Task2::stepFour, 1000);
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
