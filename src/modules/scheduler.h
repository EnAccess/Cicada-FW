/*
 * E-Lib
 * Copyright (C) 2019 EnAccess
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef ESCHEDULER_H
#define ESCHEDULER_H

#include "ctask.h"

namespace EnAccess {

/*!
 * \class Scheduler
 *
 * Simple cooperative scheduler to run instances of Task. To use Scheduler and Task,
 * do the following:
 *
 * 1. Inherit from Task and implement run(), which contains the actual code
 * you want to execute in regular intervals.
 * 2. Create a NULL-terminated array with pointers to your tasks:
 * ```
 * Task* taskList[] = { &task1, &task2, NULL };
 * ```
 * 3. Create a Scheduler, which takes the system tick function (usually the global
 * eTickFunction()) and the task list as parameters in the constructor:
 * ```
 * Scheduler s(&eTickFunction, taskList);
 * ```
 * 4. Call `s.start()` to run the main loop. This function runs in an indefinite
 * loop and never returns. Alternatively, you can also call `s.runTask()`
 * in your own loop.
 */

class Scheduler
{
  public:
    /*!
     * \param tickFunction pointer to a function returning the current
     * system time tick
     * \param taskList NULL-Terminated list of pointers to tasks
     * for being handeled by the task scheduler
     */
    Scheduler(E_TICK_TYPE (*tickFunction)(), Task* taskList[]);

    /*!
     * Check one task in the task list and if its due,
     * call it's run method.
     */
    void runTask();

    /*!
     * Starts the scheduler. The method simply calls runTask()
     * in a loop.
     */
    void start();

  private:
    E_TICK_TYPE (*_tickFunction)();
    Task** _taskList;
    Task** _currentTask;
};
}

#endif
