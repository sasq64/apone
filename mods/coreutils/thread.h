#ifndef APONE_THREAD_H
#define APONE_THREAD_H

#ifdef WINDOWS
#include <windows.h>
#include "mingw-std-threads/mingw.thread.h"
#include "mingw-std-threads/mingw.mutex.h"
#endif
#include <thread>
#include <mutex>

#endif // APONE_THREAD_H
