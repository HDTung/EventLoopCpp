//
// Created by HDTung on 8/30/20.
//

#include "Fault.h"
#include <assert.h>

#if WIN32
    #include "window.h"
#endif

/*
 * Fault Handler
 */
void FaultHandler(const char* file, unsigned short line)
{
#if WIN32
    DebugBreak();
#endif
    assert(0);
}