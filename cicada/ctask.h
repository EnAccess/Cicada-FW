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

/*!
 * \file ctask.h
 * Task class and macros.
 */

#ifndef ETASK_H
#define ETASK_H

#include <stdint.h>
#include "cicada/defines.h"

/*!
 * \def E_BEGIN_TASK
 * Use this at the beginning of run() to setup the
 * E_REENTER macros.
 */
#define E_BEGIN_TASK E_BEGIN_TASK_ARG(__COUNTER__)
#define E_BEGIN_TASK_ARG(FIRST_ENTRY)           \
    static uint8_t entrypoint = FIRST_ENTRY;    \
    switch (entrypoint) {                       \
    case FIRST_ENTRY:

/*!
 * \def E_END_TASK
 * Use this at the end of run() to terminate the
 * E_REENTER macros.
 */
#define E_END_TASK E_END_TASK_ARG(__COUNTER__)
#define E_END_TASK_ARG(LAST_ENTRY)              \
    entrypoint = LAST_ENTRY;                    \
    break; }

/*!
 * \def E_REENTER_YIELD()
 * Yields to the task scheduler, allowing other tasks to run.
 * Delay setting is not touched, it will used the delay
 * last set by setDelay() or one of the other macros.
 */
#define E_REENTER_YIELD() E_REENTER_YIELD_ARG(__COUNTER__)
#define E_REENTER_YIELD_ARG(ENTRY_POINT)        \
    entrypoint = ENTRY_POINT;                   \
    return;                                     \
    case ENTRY_POINT:

/*!
 * \def E_REENTER_DELAY(DELAY)
 * Yields to the task scheduler, allowing other tasks to run.
 * This is a convenience macro, which calls setDelay first, and
 * then does the same as E_REENTER_YIELD().
 * \param DELAY Minimum delay after which the task scheduler
 * will call run() again.
 */
#define E_REENTER_DELAY(DELAY) E_REENTER_DELAY_ARG(__COUNTER__, DELAY)
#define E_REENTER_DELAY_ARG(ENTRY_POINT, DELAY)         \
    setDelay(DELAY);                                    \
    entrypoint = ENTRY_POINT;                           \
    return;                                             \
    case ENTRY_POINT:

/*!
 * \def E_REENTER_COND(COND)
 * Continues if the condition is met, otherwise yields to the task
 * scheduler, allowing other tasks to run. The tasks delay is set to
 * 0 first, so run() will continue as soon as possible if the
 * condition is met.
 * \param COND Condition to be met to continue
 */
#define E_REENTER_COND(COND) E_REENTER_COND_ARG(__COUNTER__, COND)
#define E_REENTER_COND_ARG(ENTRY_POINT, COND)           \
    setDelay(0);                                        \
    entrypoint = ENTRY_POINT;                           \
    case ENTRY_POINT:                                   \
    if (!(COND)) return;                                \

/*!
 * \def E_REENTER_COND_DELAY(COND, DELAY)
 * Sets the delay and continues if the condition is met,
 * otherwise yields to the task scheduler. Does the same as
 * E_REENTER_COND(), but sets the delay to a user defined
 * value instead of zero.
 * \param COND Condition to be met to continue
 * \param DELAY Minimum delay after which the task scheduler
 * will call run() again.
 */
#define E_REENTER_COND_DELAY(COND, DELAY)                       \
    E_REENTER_COND_DELAY_ARG(__COUNTER__, COND, DELAY)
#define E_REENTER_COND_DELAY_ARG(ENTRY_POINT, COND, DELAY)      \
    setDelay(DELAY);                                            \
    entrypoint = ENTRY_POINT;                                   \
    case ENTRY_POINT:                                           \
    if (!(COND)) return;                                        \

namespace Cicada {

/*!
 * \class Task
 * Base class for tasks which need to be called in regular intervals.
 * To create a tasks, inherit from this class and implement it's run()
 * function, which contains the code to be called in regular intervals.
 * To actually call the run() function in regular intervals, use the
 * Scheduler class.
 * To ease state machine creation, a series of macros in ctask.h
 * can assist creating the required switch/case code.
 *
 * \see ctask.h
 *
 * Here is a simple example of a Task's run() implementation:
 * ```
 * virtual void run()
 * {
 *     E_BEGIN_TASK
 *
 *     // Do something
 *     printf("Doing something\n");
 *
 *     // Return from run, reenter here after 1 second
 *     E_REENTER_DELAY(1000);
 *
 *     // After a second, so something else
 *     printf("Doing something else\n");
 *
 *     E_END_TASK
 * }
 * ```
 */

class Task
{
  public:
    Task(uint16_t initialDelay = 0) :
        _delay(initialDelay),
        _lastRun(0)
    { }

    virtual ~Task() {}

    /*!
     * Minimum delay before the task will run again.
     */
    inline uint16_t delay() const
    {
        return _delay;
    }

    /*!
     * Set the minimum delay before the task will run again.
     * \param delay New delay
     */
    inline void setDelay(uint16_t delay)
    {
        _delay = delay;
    }

    /*!
     * Returns the time when this task was last executed
     * \return Time when this task was last executed
     */

    inline E_TICK_TYPE lastRun()
    {
        return _lastRun;
    }

    /*!
     * Update tick of last run with the global tick timer
     */
    inline void setLastRun(E_TICK_TYPE time)
    {
        _lastRun = time;
    }

    /*!
     * The starting point for the task. The scheduler will call
     * this function regularly.
     *
     * You should implement this function with the actual task.
     * The E_REENTER makros can be used to yield control to the
     * scheduler and allow other tasks to run. Use E_BEGIN_TASK
     * directly at the beginneing of run(), and E_END_TASK at
     * the end respectively.
     */
    virtual void run() = 0;

  private:
    /*
     * Doesn't make sense to copy an Task object
     */
    Task(const Task&);

    uint16_t _delay; /**< Time before the task will run again */
    E_TICK_TYPE _lastRun; /**< Stores the tick when the task last ran */
};

}

#endif
