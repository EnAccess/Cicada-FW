/*******************************************************************************
 * Copyright (c) 2013, 2014 
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Sam Grove - initial API and implementation and/or initial documentation
 *    Ian Craggs - added attached and detached member functions
 *    Sam Grove - removed need for FP.cpp
 *******************************************************************************/

#ifndef FP_H
#define FP_H

/** Example using the FP Class with global functions
 * @code
 *  #include "mbed.h"
 *  #include "FP.h"
 *
 *  FP<void,bool>fp;
 *  DigitalOut myled(LED1);
 *
 *  void handler(bool value)
 *  {
 *      myled = value;
 *      return;
 *  }
 *
 *  int main()
 *  {
 *      fp.attach(&handler);
 *
 *      while(1)
 *      {
 *          fp(1);
 *          wait(0.2);
 *          fp(0);
 *          wait(0.2);
 *      }
 *  }
 * @endcode
 */

/** Example using the FP Class with different class member functions
 * @code
 *  #include "mbed.h"
 *  #include "FP.h"
 *
 *  FP<void,bool>fp;
 *  DigitalOut myled(LED4);
 *
 *  class Wrapper
 *  {
 *  public:
 *      Wrapper(){}
 *
 *      void handler(bool value)
 *      {
 *          myled = value;
 *          return;
 *      }
 *  };
 *
 *  int main()
 *  {
 *      Wrapper wrapped;
 *      fp.attach(&wrapped, &Wrapper::handler);
 *
 *      while(1)
 *      {
 *          fp(1);
 *          wait(0.2);
 *          fp(0);
 *          wait(0.2);
 *      }
 *  }
 * @endcode
 */

/** Example using the FP Class with member FP and member function
* @code
*  #include "mbed.h"
*  #include "FP.h"
*
*  DigitalOut myled(LED2);
*
*  class Wrapper
*  {
*  public:
*      Wrapper()
*      {
*          fp.attach(this, &Wrapper::handler);
*      }
*
*      void handler(bool value)
*      {
*          myled = value;
*          return;
*      }
*
*      FP<void,bool>fp;
*  };
*
*  int main()
*  {
*      Wrapper wrapped;
*
*      while(1)
*      {
*          wrapped.fp(1);
*          wait(0.2);
*          wrapped.fp(0);
*          wait(0.2);
*      }
*  }
* @endcode
*/

/**
 *  @class FP
 *  @brief API for managing Function Pointers
 */
template<class retT>
class FP
{
public:
    /** Create the FP object - only one callback can be attached to the object, that is
     *  a member function or a global function, not both at the same time
     */
    template<class T>
    FP(T item)
    {
        obj_callback = (FPtrDummy *)(item);
        c_callback = 0;
    }

    FP()
    {
        obj_callback = 0;
        c_callback = 0;
    }

    /** Add a callback function to the object
     *  @param item - Address of the initialized object
     *  @param member - Address of the member function (dont forget the scope that the function is defined in)
     */
    template<class T>
    void attach(T *item, retT (T::*method)())
    {
        obj_callback = (FPtrDummy *)(item);
        method_callback = (retT (FPtrDummy::*)())(method);
        return;
    }

    template<class T>
    void attach(retT (T::*method)())
    {
        method_callback = (retT (FPtrDummy::*)())(method);
        return;
    }

    /** Add a callback function to the object
     *  @param function - The address of a globally defined function
     */
    // void attach(retT (*function)())
    // {
    //     c_callback = function;
    // }

    /** Invoke the function attached to the class
     *  @param arg - An argument that is passed into the function handler that is called
     *  @return The return from the function hanlder called by this class
     */
    retT operator()() const
    {
        return run();
    }

    retT run() const
    {
        if( 0 != c_callback ) {
            return obj_callback ? (obj_callback->*method_callback)() : (*c_callback)();
        }
        return (retT)0;
    }

    /** Determine if an callback is currently hooked
     *  @return 1 if a method is hooked, 0 otherwise
     */
    bool attached()
    {
        return c_callback;
    }

    /** Release a function from the callback hook
     */
    void detach()
    {
        obj_callback = 0;
        c_callback = 0;
    }

private:

    // empty type used for casting
    class FPtrDummy;

    FPtrDummy *obj_callback;

    /**
     *  @union Funciton
     *  @brief Member or global callback function
     */
    union {
        retT (*c_callback)();                   /*!< Footprint for a global function */
        retT (FPtrDummy::*method_callback)();   /*!< Footprint for a member function */
    };
};

#endif