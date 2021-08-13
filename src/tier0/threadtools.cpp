//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================

#include "pch_tier0.h"

#include "tier1/strtools.h"
#include "tier0/dynfunction.h"
#if defined( _WIN32 ) && !defined( _X360 )
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <synchapi.h>
#endif
#ifdef _WIN32
	#include <process.h>
	
#ifdef IS_WINDOWS_PC	
	#include <mmsystem.h>
	#pragma comment(lib, "winmm.lib")
#endif // IS_WINDOWS_PC

#elif defined(POSIX)

#if !defined(OSX)
	#include <sys/fcntl.h>
	#include <sys/unistd.h>
	#define sem_unlink( arg )
	#define OS_TO_PTHREAD(x) (x)
#else
	#define pthread_yield pthread_yield_np
	#include <mach/thread_act.h>
	#include <mach/mach.h>
	#define OS_TO_PTHREAD(x) pthread_from_mach_thread_np( x )
#endif // !OSX

#ifdef LINUX
#include <dlfcn.h> // RTLD_NEXT
#endif

typedef int (*PTHREAD_START_ROUTINE)(
    void *lpThreadParameter
    );
typedef PTHREAD_START_ROUTINE LPTHREAD_START_ROUTINE;
#include <sched.h>
#include <exception>
#include <errno.h>
#include <signal.h>
#include <pthread.h>
#include <sys/time.h>
#define GetLastError() errno
typedef void *LPVOID;
#endif

#include "tier0/valve_minmax_off.h"
#include <memory>
#include "tier0/valve_minmax_on.h"

#include "tier0/threadtools.h"
#include "tier0/vcrmode.h"

#ifdef _X360
#include "xbox/xbox_win32stubs.h"
#endif

#include "tier0/vprof_telemetry.h"

// Must be last header...
#include "tier0/memdbgon.h"

//#define THREADS_DEBUG 1

// Need to ensure initialized before other clients call in for main thread ID
#ifdef _WIN32
#pragma warning(disable:4073)
#pragma init_seg(lib)
#endif

#ifdef _WIN32
ASSERT_INVARIANT(TT_SIZEOF_CRITICALSECTION == sizeof(CRITICAL_SECTION));
ASSERT_INVARIANT(TT_INFINITE == INFINITE);
#endif

//-----------------------------------------------------------------------------
// Simple thread functions. 
// Because _beginthreadex uses stdcall, we need to convert to cdecl
//-----------------------------------------------------------------------------
struct ThreadProcInfo_t
{
	ThreadProcInfo_t( ThreadFunc_t pfnThread, void *pParam )
	  : pfnThread( pfnThread),
		pParam( pParam )
	{
	}
	
	ThreadFunc_t pfnThread;
	void *		 pParam;
};

//---------------------------------------------------------

#ifdef _WIN32
static unsigned __stdcall ThreadProcConvert( void *pParam )
#elif defined(POSIX)
static void *ThreadProcConvert( void *pParam )
#else
#error
#endif
{
	ThreadProcInfo_t info = *((ThreadProcInfo_t *)pParam);
	delete ((ThreadProcInfo_t *)pParam);
#ifdef _WIN32
	return (*info.pfnThread)(info.pParam);
#elif defined(POSIX)
	return (void *)(*info.pfnThread)(info.pParam);
#else
#error
#endif
}


//---------------------------------------------------------

ThreadHandle_t CreateSimpleThread( ThreadFunc_t pfnThread, void *pParam, ThreadId_t *pID, unsigned stackSize )
{
#ifdef _WIN32
	ThreadId_t idIgnored;
	if ( !pID )
		pID = &idIgnored;
	HANDLE h = VCRHook_CreateThread(NULL, stackSize, (LPTHREAD_START_ROUTINE)ThreadProcConvert, new ThreadProcInfo_t( pfnThread, pParam ), CREATE_SUSPENDED, pID);
	if ( h != INVALID_HANDLE_VALUE )
	{
		Plat_ApplyHardwareDataBreakpointsToNewThread( *pID );
		ResumeThread( h );
	}
	return (ThreadHandle_t)h;
#elif defined(POSIX)
	pthread_t tid;

	// If we need to create threads that are detached right out of the gate, we would need to do something like this:
	//   pthread_attr_t attr;
	//   int rc = pthread_attr_init(&attr);
	//   rc = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	//   ... pthread_create( &tid, &attr, ... ) ...
	//   rc = pthread_attr_destroy(&attr);
	//   ... pthread_join will now fail

	int ret = pthread_create( &tid, NULL, ThreadProcConvert, new ThreadProcInfo_t( pfnThread, pParam ) );
	if ( ret )
	{
		// There are only PTHREAD_THREADS_MAX number of threads, and we're probably leaking handles if ret == EAGAIN here?
		Error( "CreateSimpleThread: pthread_create failed. Someone not calling pthread_detach() or pthread_join. Ret:%d\n", ret );
	}
	if ( pID )
		*pID = (ThreadId_t)tid;
	Plat_ApplyHardwareDataBreakpointsToNewThread( (long unsigned int)tid );
	return (ThreadHandle_t)tid;
#endif
}

ThreadHandle_t CreateSimpleThread( ThreadFunc_t pfnThread, void *pParam, unsigned stackSize )
{
	return CreateSimpleThread( pfnThread, pParam, NULL, stackSize );
}

PLATFORM_INTERFACE void ThreadDetach( ThreadHandle_t hThread )
{
#if defined( POSIX )
	// The resources of this thread will be freed immediately when it terminates,
	//  instead of waiting for another thread to perform PTHREAD_JOIN.
	pthread_t tid = ( pthread_t )hThread;

	pthread_detach( tid );
#endif
}

bool ReleaseThreadHandle( ThreadHandle_t hThread )
{
#ifdef _WIN32
	return ( CloseHandle( hThread ) != 0 );
#else
	return true;
#endif
}

//-----------------------------------------------------------------------------
//
// Wrappers for other simple threading operations
//
//-----------------------------------------------------------------------------

void ThreadSleep(unsigned nMilliseconds)
{
#ifdef _WIN32

#ifdef IS_WINDOWS_PC
	static bool bInitialized = false;
	if ( !bInitialized )
	{
		bInitialized = true;
		// Set the timer resolution to 1 ms (default is 10.0, 15.6, 2.5, 1.0 or
		// some other value depending on hardware and software) so that we can
		// use Sleep( 1 ) to avoid wasting CPU time without missing our frame
		// rate.
		timeBeginPeriod( 1 );
	}
#endif // IS_WINDOWS_PC

#if 1
	if ( nMilliseconds > 0 )
	{
		SleepEx( nMilliseconds, true );
	}
	else
	{
		SwitchToThread();
	}
#else
	if (nMilliseconds != 0 || !SwitchToThread())
	{
		SleepEx( nMilliseconds, true );
	}
#endif
#elif defined(POSIX)
	if ( nMilliseconds > 0 )
	{
		usleep(nMilliseconds * 1000);
	}
	else
	{
		sched_yield();
	}
#endif
}

void ThreadSleepEx(unsigned nMilliseconds)
{
	// hint to CPU that we're spin waiting
	ThreadPause();
#ifdef _WIN32

#ifdef IS_WINDOWS_PC
	static bool bInitialized = false;
	if (!bInitialized)
	{
		bInitialized = true;
		// Set the timer resolution to 1 ms (default is 10.0, 15.6, 2.5, 1.0 or
		// some other value depending on hardware and software) so that we can
		// use Sleep( 1 ) to avoid wasting CPU time without missing our frame
		// rate.
		timeBeginPeriod(1);
	}
#endif // IS_WINDOWS_PC
	SleepEx(nMilliseconds, true);
#elif defined(POSIX)
	usleep(nMilliseconds * 1000);
#endif
}

// The .NET Foundation licenses this to you under the MIT license.
// Defaults are for when InitializeYieldProcessorNormalized has not yet been called or when no measurement is done, and are
// tuned for Skylake processors
unsigned int g_yieldsPerNormalizedYield = 1; // current value is for Skylake processors, this is expected to be ~8 for pre-Skylake
unsigned int g_optimalMaxNormalizedYieldsPerSpinIteration = 7;

const unsigned int MinNsPerNormalizedYield = 37; // measured typically 37-46 on post-Skylake
const unsigned int NsPerOptimalMaxSpinIterationDuration = 272; // approx. 900 cycles, measured 281 on pre-Skylake, 263 on post-Skylake
bool s_isYieldProcessorNormalizedInitialized = false;

#ifdef POSIX
void InitThreadSpinCount()
{
    // TODO: Implement
    s_isYieldProcessorNormalizedInitialized = true;
}
#endif

#ifdef WIN32
void InitThreadSpinCount()
{
	if (s_isYieldProcessorNormalizedInitialized)
	{
		return;
	}

	// Intel pre-Skylake processor: measured typically 14-17 cycles per yield
	// Intel post-Skylake processor: measured typically 125-150 cycles per yield
	const int MeasureDurationMs = 10;
	const int NsPerSecond = 1000 * 1000 * 1000;

	LARGE_INTEGER li;
	if (!QueryPerformanceFrequency(&li) || (ULONGLONG)li.QuadPart < 1000 / MeasureDurationMs)
	{
		// High precision clock not available or clock resolution is too low, resort to defaults
		s_isYieldProcessorNormalizedInitialized = true;
		return;
	}
	ULONGLONG ticksPerSecond = li.QuadPart;

	// Measure the nanosecond delay per yield
	ULONGLONG measureDurationTicks = ticksPerSecond / (1000 / MeasureDurationMs);
	unsigned int yieldCount = 0;
	QueryPerformanceCounter(&li);
	ULONGLONG startTicks = li.QuadPart;
	ULONGLONG elapsedTicks;
	do
	{
		// On some systems, querying the high performance counter has relatively significant overhead. Do enough yields to mask
		// the timing overhead. Assuming one yield has a delay of MinNsPerNormalizedYield, 1000 yields would have a delay in the
		// low microsecond range.
		for (int i = 0; i < 1000; ++i)
		{
			ThreadPause();
		}
		yieldCount += 1000;

		QueryPerformanceCounter(&li);
		ULONGLONG nowTicks = li.QuadPart;
		elapsedTicks = nowTicks - startTicks;
	} while (elapsedTicks < measureDurationTicks);
	double nsPerYield = (double)elapsedTicks * NsPerSecond / ((double)yieldCount * ticksPerSecond);
	if (nsPerYield < 1)
	{
		nsPerYield = 1;
	}

	// Calculate the number of yields required to span the duration of a normalized yield. Since nsPerYield is at least 1, this
	// value is naturally limited to MinNsPerNormalizedYield.
	int yieldsPerNormalizedYield = (int)(MinNsPerNormalizedYield / nsPerYield + 0.5);
	if (yieldsPerNormalizedYield < 1)
	{
		yieldsPerNormalizedYield = 1;
	}
	_ASSERTE(yieldsPerNormalizedYield <= (int)MinNsPerNormalizedYield);

	// Calculate the maximum number of yields that would be optimal for a late spin iteration. Typically, we would not want to
	// spend excessive amounts of time (thousands of cycles) doing only YieldProcessor, as SwitchToThread/Sleep would do a
	// better job of allowing other work to run.
	int optimalMaxNormalizedYieldsPerSpinIteration =
		(int)(NsPerOptimalMaxSpinIterationDuration / (yieldsPerNormalizedYield * nsPerYield) + 0.5);
	if (optimalMaxNormalizedYieldsPerSpinIteration < 1)
	{
		optimalMaxNormalizedYieldsPerSpinIteration = 1;
	}

	g_yieldsPerNormalizedYield = yieldsPerNormalizedYield;
	g_optimalMaxNormalizedYieldsPerSpinIteration = optimalMaxNormalizedYieldsPerSpinIteration;
	s_isYieldProcessorNormalizedInitialized = true;
}
#endif

// This assumes a Skylake tuned iteration count.
void ThreadSpin(unsigned iterations)
{
	if (iterations <= 0)
	{
		return;
	}

	if (!s_isYieldProcessorNormalizedInitialized)
	{
		InitThreadSpinCount();
	}

	unsigned n = iterations * g_yieldsPerNormalizedYield;
	do
	{
		ThreadPause();
	} while (--n != 0);
}

//-----------------------------------------------------------------------------

#ifndef ThreadGetCurrentId
uint ThreadGetCurrentId()
{
#ifdef _WIN32
	return GetCurrentThreadId();
#elif defined(POSIX)
	return (uint)pthread_self();
#endif
}
#endif

//-----------------------------------------------------------------------------
ThreadHandle_t ThreadGetCurrentHandle()
{
#ifdef _WIN32
	return (ThreadHandle_t)GetCurrentThread();
#elif defined(POSIX)
	return (ThreadHandle_t)pthread_self();
#endif
}

// On PS3, this will return true for zombie threads
bool ThreadIsThreadIdRunning( ThreadId_t uThreadId )
{
#ifdef _WIN32
	bool bRunning = true;
	HANDLE hThread = ::OpenThread( THREAD_QUERY_INFORMATION , false, uThreadId );
	if ( hThread )
	{
		DWORD dwExitCode;
		if( !::GetExitCodeThread( hThread, &dwExitCode ) || dwExitCode != STILL_ACTIVE )
			bRunning = false;

		CloseHandle( hThread );
	}
	else
	{
		bRunning = false;
	}
	return bRunning;
#elif defined( _PS3 )
	
	// will return CELL_OK for zombie threads
	int priority;
	return (sys_ppu_thread_get_priority( uThreadId, &priority ) == CELL_OK );

#elif defined(POSIX)
	pthread_t thread = OS_TO_PTHREAD(uThreadId);
	if ( thread )
	{
		int iResult = pthread_kill( thread, 0 );
		if ( iResult == 0 )
			return true;
	}
	else
	{
		// We really ought not to be passing NULL in to here
		AssertMsg( false, "ThreadIsThreadIdRunning received a null thread ID" );
	}

	return false;
#endif
}

//-----------------------------------------------------------------------------

int ThreadGetPriority( ThreadHandle_t hThread )
{
	if ( !hThread )
	{
		hThread = ThreadGetCurrentHandle();
	}

#ifdef _WIN32
	return ::GetThreadPriority( (HANDLE)hThread );
#else
	struct sched_param thread_param;
	int policy;
	pthread_getschedparam( (pthread_t)hThread, &policy, &thread_param );
	return thread_param.sched_priority;
#endif
}

//-----------------------------------------------------------------------------

bool ThreadSetPriority( ThreadHandle_t hThread, int priority )
{
	if ( !hThread )
	{
		hThread = ThreadGetCurrentHandle();
	}

#ifdef _WIN32
	return ( SetThreadPriority(hThread, priority) != 0 );
#elif defined(POSIX)
	struct sched_param thread_param; 
	thread_param.sched_priority = priority; 
	pthread_setschedparam( (pthread_t)hThread, SCHED_OTHER, &thread_param );
	return true;
#endif
}

//-----------------------------------------------------------------------------

void ThreadSetAffinity( ThreadHandle_t hThread, int nAffinityMask )
{
#if USE_AFFINITY
	if ( !hThread )
	{
		hThread = ThreadGetCurrentHandle();
	}

#ifdef _WIN32
	SetThreadAffinityMask( hThread, nAffinityMask );
#elif defined(POSIX)
// 	cpu_set_t cpuSet;
// 	CPU_ZERO( cpuSet );
// 	for( int i = 0 ; i < 32; i++ )
// 	  if ( nAffinityMask & ( 1 << i ) )
// 	    CPU_SET( cpuSet, i );
// 	sched_setaffinity( hThread, sizeof( cpuSet ), &cpuSet );
#endif
#endif
}

//-----------------------------------------------------------------------------

uint InitMainThread()
{
#ifndef LINUX
	// Skip doing the setname on Linux for the main thread. Here is why...

	// From Pierre-Loup e-mail about why pthread_setname_np() on the main thread
	// in Linux will cause some tools to display "MainThrd" as the executable name:
	//
	// You have two things in procfs, comm and cmdline.  Each of the threads have
	// a different `comm`, which is the value you set through pthread_setname_np
	// or prctl(PR_SET_NAME).  Top can either display cmdline or comm; it
	// switched to display comm by default; htop still displays cmdline by
	// default. Top -c will output cmdline rather than comm.
	// 
	// If you press 'H' while top is running it will display each thread as a
	// separate process, so you will have different entries for MainThrd,
	// MatQueue0, etc with their own CPU usage.  But when that mode isn't enabled
	// it just displays the 'comm' name from the first thread.
	ThreadSetDebugName( "MainThrd" );
#endif

#ifdef _WIN32
	return ThreadGetCurrentId();
#elif defined(POSIX)
	return (uint)pthread_self();
#endif
}

uint g_ThreadMainThreadID = InitMainThread();

bool ThreadInMainThread()
{
	return ( ThreadGetCurrentId() == g_ThreadMainThreadID );
}

//-----------------------------------------------------------------------------
void DeclareCurrentThreadIsMainThread()
{
	g_ThreadMainThreadID = ThreadGetCurrentId();
}

bool ThreadJoin( ThreadHandle_t hThread, unsigned timeout )
{
	// You should really never be calling this with a NULL thread handle. If you
	// are then that probably implies a race condition or threading misunderstanding.
	Assert( hThread );
	if ( !hThread )
	{
		return false;
	}

#ifdef _WIN32
	DWORD dwWait = VCRHook_WaitForSingleObject((HANDLE)hThread, timeout);
	if ( dwWait == WAIT_TIMEOUT)
		return false;
	if ( dwWait != WAIT_OBJECT_0 && ( dwWait != WAIT_FAILED && GetLastError() != 0 ) )
	{
		Assert( 0 );
		return false;
	}
#elif defined(POSIX)
	if ( pthread_join( (pthread_t)hThread, NULL ) != 0 )
		return false;
#endif
	return true;
}

#ifdef RAD_TELEMETRY_ENABLED
void TelemetryThreadSetDebugName( ThreadId_t id, const char *pszName );
#endif

//-----------------------------------------------------------------------------

void ThreadSetDebugName( ThreadId_t id, const char *pszName )
{
	if( !pszName )
		return;

#ifdef RAD_TELEMETRY_ENABLED
	TelemetryThreadSetDebugName( id, pszName );
#endif

#ifdef _WIN32
	if ( Plat_IsInDebugSession() )
	{
#define MS_VC_EXCEPTION 0x406d1388

		typedef struct tagTHREADNAME_INFO
		{
			DWORD dwType;        // must be 0x1000
			LPCSTR szName;       // pointer to name (in same addr space)
			DWORD dwThreadID;    // thread ID (-1 caller thread)
			DWORD dwFlags;       // reserved for future use, most be zero
		} THREADNAME_INFO;

		THREADNAME_INFO info;
		info.dwType = 0x1000;
		info.szName = pszName;
		info.dwThreadID = id;
		info.dwFlags = 0;

		__try
		{
			RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(DWORD), (ULONG_PTR *)&info);
		}
		__except (EXCEPTION_CONTINUE_EXECUTION)
		{
		}
	}
#elif defined( _LINUX )
	// As of glibc v2.12, we can use pthread_setname_np.
	typedef int (pthread_setname_np_func)(pthread_t, const char *);
	static pthread_setname_np_func *s_pthread_setname_np_func = (pthread_setname_np_func *)dlsym(RTLD_DEFAULT, "pthread_setname_np");

	if ( s_pthread_setname_np_func )
	{
		if ( id == (uint32)-1 )
			id = pthread_self();

		/* 
			pthread_setname_np() in phthread_setname.c has the following code:
		
			#define TASK_COMM_LEN 16
			  size_t name_len = strlen (name);
			  if (name_len >= TASK_COMM_LEN)
				return ERANGE;
		
			So we need to truncate the threadname to 16 or the call will just fail.
		*/
		char szThreadName[ 16 ];
		strncpy( szThreadName, pszName, ARRAYSIZE( szThreadName ) );
		szThreadName[ ARRAYSIZE( szThreadName ) - 1 ] = 0;
		(*s_pthread_setname_np_func)( id, szThreadName );
	}
#endif
}

//-----------------------------------------------------------------------------

#ifdef _WIN32
ASSERT_INVARIANT( TW_FAILED == WAIT_FAILED );
ASSERT_INVARIANT( TW_TIMEOUT  == WAIT_TIMEOUT );
ASSERT_INVARIANT( WAIT_OBJECT_0 == 0 );

int ThreadWaitForObjects( int nEvents, const HANDLE *pHandles, bool bWaitAll, unsigned timeout )
{
	return VCRHook_WaitForMultipleObjects( nEvents, pHandles, bWaitAll, timeout );
}
#endif


//-----------------------------------------------------------------------------
// Used to thread LoadLibrary on the 360
//-----------------------------------------------------------------------------
static ThreadedLoadLibraryFunc_t s_ThreadedLoadLibraryFunc = 0;
PLATFORM_INTERFACE void SetThreadedLoadLibraryFunc( ThreadedLoadLibraryFunc_t func )
{
	s_ThreadedLoadLibraryFunc = func;
}

PLATFORM_INTERFACE ThreadedLoadLibraryFunc_t GetThreadedLoadLibraryFunc()
{
	return s_ThreadedLoadLibraryFunc;
}


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

CThreadSyncObject::CThreadSyncObject()
#ifdef _WIN32
  : m_hSyncObject( NULL ), m_bCreatedHandle(false)
#elif defined(POSIX)
  : m_bInitalized( false )
#endif
{
}

//---------------------------------------------------------

CThreadSyncObject::~CThreadSyncObject()
{
#ifdef _WIN32
   if ( m_hSyncObject && m_bCreatedHandle )
   {
      if ( !CloseHandle(m_hSyncObject) )
	  {
		  Assert( 0 );
	  }
   }
#elif defined(POSIX)
   if ( m_bInitalized )
   {
	pthread_cond_destroy( &m_Condition );
        pthread_mutex_destroy( &m_Mutex );
	m_bInitalized = false;
   }
#endif
}

//---------------------------------------------------------

bool CThreadSyncObject::operator!() const
{
#ifdef _WIN32
   return !m_hSyncObject;
#elif defined(POSIX)
   return !m_bInitalized;
#endif
}

//---------------------------------------------------------

void CThreadSyncObject::AssertUseable()
{
#ifdef THREADS_DEBUG
#ifdef _WIN32
   AssertMsg( m_hSyncObject, "Thread synchronization object is unuseable" );
#elif defined(POSIX)
   AssertMsg( m_bInitalized, "Thread synchronization object is unuseable" );
#endif
#endif
}

//---------------------------------------------------------

bool CThreadSyncObject::Wait( uint32 dwTimeout )
{
#ifdef THREADS_DEBUG
   AssertUseable();
#endif
#ifdef _WIN32
   return ( VCRHook_WaitForSingleObject( m_hSyncObject, dwTimeout ) == WAIT_OBJECT_0 );
#elif defined(POSIX)
    pthread_mutex_lock( &m_Mutex );
    bool bRet = false;
    if ( m_cSet > 0 )
    {
		bRet = true;
		m_bWakeForEvent = false;
    }
    else
    {
		volatile int ret = 0;

		while ( !m_bWakeForEvent && ret != ETIMEDOUT )
		{
			struct timeval tv;
			gettimeofday( &tv, NULL );
			volatile struct timespec tm;
			
			uint64 actualTimeout = dwTimeout;
			
			if ( dwTimeout == TT_INFINITE && m_bManualReset )
				actualTimeout = 10; // just wait 10 msec at most for manual reset events and loop instead
				
			volatile uint64 nNanoSec = (uint64)tv.tv_usec*1000 + (uint64)actualTimeout*1000000;
			tm.tv_sec = tv.tv_sec + nNanoSec /1000000000;
			tm.tv_nsec = nNanoSec % 1000000000;

			do
			{   
				ret = pthread_cond_timedwait( &m_Condition, &m_Mutex, (const timespec *)&tm );
			} 
			while( ret == EINTR );

			bRet = ( ret == 0 );
			
			if ( m_bManualReset )
			{
				if ( m_cSet )
					break;
				if ( dwTimeout == TT_INFINITE && ret == ETIMEDOUT )
					ret = 0; // force the loop to spin back around
			}
		}
		
		if ( bRet )
			m_bWakeForEvent = false;
    }
    if ( !m_bManualReset && bRet )
		m_cSet = 0;
    pthread_mutex_unlock( &m_Mutex );
    return bRet;
#endif
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

//---------------------------------------------------------

void CStdThreadSyncObject::AssertUseable()
{
#ifdef THREADS_DEBUG
	AssertMsg(m_bInitialized, "Thread synchronization object is unusable");
#endif
}

//---------------------------------------------------------

bool CStdThreadSyncObject::Wait( uint32 dwTimeout )
{
#ifdef THREADS_DEBUG
    AssertUseable();
#endif
    bool bRet = m_bSignaled.load(std::memory_order::memory_order_consume);
#if 1
	if (!bRet && dwTimeout != 0)
#else
	if (bRet || dwTimeout == 0)
	{
	    // Emulate context switch behavior seen in other waits
		ThreadSleep(0);
	}
	else
#endif
	{
		std::unique_lock lock(m_Mutex);
		if (dwTimeout == TT_INFINITE)
        {
	        m_Condition.wait(lock, [this] { return m_bSignaled.load(std::memory_order::memory_order_consume); });
	        bRet = true;
        }
        else
        {
	        bRet = m_Condition.wait_for(lock, std::chrono::milliseconds(dwTimeout), [this] { return m_bSignaled.load(std::memory_order::memory_order_consume); });
        }
	}
    if (m_bAutoReset && bRet)
    {
	    m_bSignaled.store(false, std::memory_order::memory_order_release);
    }
    return bRet;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

CThreadEvent::CThreadEvent( bool bManualReset )
{
#ifdef _WIN32
    m_hSyncObject = CreateEvent( NULL, bManualReset, FALSE, NULL );
	m_bCreatedHandle = true;
    AssertMsg1(m_hSyncObject, "Failed to create event (error 0x%x)", GetLastError() );
#elif defined( POSIX )
    pthread_mutexattr_t Attr;
    pthread_mutexattr_init( &Attr );
    pthread_mutex_init( &m_Mutex, &Attr );
    pthread_mutexattr_destroy( &Attr );
    pthread_cond_init( &m_Condition, NULL );
    m_bInitalized = true;
    m_cSet = 0;
	m_bWakeForEvent = false;
    m_bManualReset = bManualReset;
#else
#error "Implement me"
#endif
}

#ifdef _WIN32
CThreadEvent::CThreadEvent( HANDLE hHandle )
{
	m_hSyncObject = hHandle;
	m_bCreatedHandle = false;
	AssertMsg(m_hSyncObject, "Null event passed into constructor" );
}
#endif

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------


//---------------------------------------------------------

bool CThreadEvent::Set()
{
   AssertUseable();
#ifdef _WIN32
   return ( SetEvent( m_hSyncObject ) != 0 );
#elif defined(POSIX)
    pthread_mutex_lock( &m_Mutex );
    m_cSet = 1;
	m_bWakeForEvent = true;
    int ret = pthread_cond_signal( &m_Condition );
    pthread_mutex_unlock( &m_Mutex );
    return ret == 0;
#endif
}

//---------------------------------------------------------

bool CThreadEvent::Reset()
{
#ifdef THREADS_DEBUG
   AssertUseable();
#endif
#ifdef _WIN32
   return ( ResetEvent( m_hSyncObject ) != 0 );
#elif defined(POSIX)
	pthread_mutex_lock( &m_Mutex );
	m_cSet = 0;
	m_bWakeForEvent = false;
	pthread_mutex_unlock( &m_Mutex );
	return true; 
#endif
}

//---------------------------------------------------------

bool CThreadEvent::Check()
{
#ifdef THREADS_DEBUG
   AssertUseable();
#endif
	return Wait( 0 );
}



bool CThreadEvent::Wait( uint32 dwTimeout )
{
	return CThreadSyncObject::Wait( dwTimeout );
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------


//---------------------------------------------------------

bool CStdThreadEvent::Set()
{
#ifdef THREADS_DEBUG
	AssertUseable();
#endif
	if (m_bSignaled.load(std::memory_order::memory_order_consume))
	{
		// We could let Set fallthrough here if the event is signaled, but it would mask race conditions.
		return true;
	}
	m_bSignaled.store(true, std::memory_order::memory_order_release);

	// Lock because we want to sync m_listeningConditions
	std::unique_lock lock(m_Mutex);
	// If we are not going to notify a listener, then we can be less pessimistic and unlock now
	// By pessimism, as we say above and in the below sections, we mean keeping the lock until the end of scope
	// when we have already notified a condition variable which will be trying to get ahold of the lock before
	// we reach the end of scope. Therefore, it's important that we unlock BEFORE we notify.
	// We have to be pessimistic otherwise because m_listeningConditions needs a lock.
	const size_t iListeners = m_listeningConditions.Size();
	const bool bNoListeners = iListeners == 0;
	if (bNoListeners)
	{
		lock.unlock();
	}

	if (m_bAutoReset)
	{
		// wait for events either holds a scoped_lock on all events (for wait all), or does a Check() which locks (for wait any)
		// so, if we get to this point, it's because WaitForEvents got a lock and should be next in line
		// this code is slightly duplicated below in the non-auto reset to avoid checking m_bAutoReset for each listener
		if (!bNoListeners)
		{
		    std::shared_ptr<std::condition_variable_any> condition;
	        while (m_listeningConditions.PopItemFront(condition))
		    {
			    if (condition)
			    {
				    // We aren't going to notify anything else, so unlock now.
				    lock.unlock();
				    condition->notify_one();
				    condition.reset();
				    // Since it's an auto reset, if we notify a listener, we've already let an event through, so don't let another one through.
				    return true;
			    }
		    }

		    // Be non-pessimistic after we loop through
			lock.unlock();
		}

		//m_Condition.notify_one();
	}
	else
	{
		// wait for events either holds a scoped_lock on all events (for wait all), or does a Check() which locks (for wait any)
		// so, if we get to this point, it's because WaitForEvents got a lock and should be next in line
		if (!bNoListeners)
		{
			const bool bOneListener = iListeners == 1;
	        std::shared_ptr<std::condition_variable_any> condition;
			if (bOneListener)
			{
				m_listeningConditions.PopItem(condition);
				lock.unlock();
				if (condition)
				{
					condition->notify_one();
					condition.reset();
				}
			}
			else
			{
				while (m_listeningConditions.PopItem(condition))
				{
					if (condition)
					{
						// TODO: unfortunately, with multiple, we have to take the pessimistic case here, since we will be notifying multiple listeners within this queue
						condition->notify_one();
						condition.reset();
					}
				}
				// At least be non-pessimistic after we loop through
				lock.unlock();
			}
		}

		//m_Condition.notify_all();
	}
	return true;
}

//---------------------------------------------------------

bool CStdThreadEvent::Reset()
{
#ifdef THREADS_DEBUG
    AssertUseable();
#endif
    m_bSignaled.store(false, std::memory_order::memory_order_release);
    return true;
}

//---------------------------------------------------------

bool CStdThreadEvent::Check()
{
#ifdef THREADS_DEBUG
    AssertUseable();
#endif
    return Wait( 0 );
}

//---------------------------------------------------------

bool CStdThreadEvent::Wait( uint32 dwTimeout )
{
#if 1
	CStdThreadEvent* pEvent = this;
	return WaitForMultiple(1, &pEvent, true, dwTimeout) != TW_TIMEOUT;
#else
	return CStdThreadSyncObject::Wait( dwTimeout );
#endif
}

#if 0
#define GENERATE_2_TO_64( INNERMACRONAME ) \
	INNERMACRONAME(2); \
	INNERMACRONAME(3); \
	INNERMACRONAME(4); \
	INNERMACRONAME(5); \
	INNERMACRONAME(6); \
	INNERMACRONAME(7); \
	INNERMACRONAME(8); \
	INNERMACRONAME(9); \
	INNERMACRONAME(10);\
	INNERMACRONAME(11);\
	INNERMACRONAME(12);\
	INNERMACRONAME(13);\
	INNERMACRONAME(14);\
	INNERMACRONAME(15);\
	INNERMACRONAME(16);\
	INNERMACRONAME(17);\
	INNERMACRONAME(18);\
	INNERMACRONAME(19);\
	INNERMACRONAME(20);\
	INNERMACRONAME(21);\
	INNERMACRONAME(22);\
	INNERMACRONAME(23);\
	INNERMACRONAME(24);\
	INNERMACRONAME(25);\
	INNERMACRONAME(26);\
	INNERMACRONAME(27);\
	INNERMACRONAME(28);\
	INNERMACRONAME(29);\
	INNERMACRONAME(30);\
	INNERMACRONAME(31);\
	INNERMACRONAME(32);\
	INNERMACRONAME(33);\
	INNERMACRONAME(34);\
	INNERMACRONAME(35);\
	INNERMACRONAME(36);\
	INNERMACRONAME(37);\
	INNERMACRONAME(38);\
	INNERMACRONAME(39);\
	INNERMACRONAME(40);\
	INNERMACRONAME(41);\
	INNERMACRONAME(42);\
	INNERMACRONAME(43);\
	INNERMACRONAME(44);\
	INNERMACRONAME(45);\
	INNERMACRONAME(46);\
	INNERMACRONAME(47);\
	INNERMACRONAME(48);\
	INNERMACRONAME(49);\
	INNERMACRONAME(50);\
	INNERMACRONAME(51);\
	INNERMACRONAME(52);\
	INNERMACRONAME(53);\
	INNERMACRONAME(54);\
	INNERMACRONAME(55);\
	INNERMACRONAME(56);\
	INNERMACRONAME(57);\
	INNERMACRONAME(58);\
	INNERMACRONAME(59);\
	INNERMACRONAME(60);\
	INNERMACRONAME(61);\
	INNERMACRONAME(62);\
	INNERMACRONAME(63);\
	INNERMACRONAME(64)

#define DEFINE_WAIT_ALL(N)

#define DEFINE_WAIT_ANY(N)
#endif

int CStdThreadEvent::WaitForMultiple(int nEvents, CStdThreadEvent* const* pEvents, bool bWaitAll, unsigned timeout)
{
	Assert(nEvents > 0);
#if 0
	if (nEvents == 1)
	{
		if (pEvents[0]->Wait(timeout))
			return WAIT_OBJECT_0;
		return TW_TIMEOUT;
	}
#endif
	bool bRet = false;
	int iEventIndex = 0;

	if (bWaitAll)
	{
		auto lPredSignaledAll = [nEvents, &pEvents]
		{
			for (int i = 0; i < nEvents; i++)
			{
				if (!pEvents[i]->m_bSignaled.load(std::memory_order::memory_order_consume))
				{
					return false;
				}
			}

			return true;
		};

		if (lPredSignaledAll())
		{
			bRet = true;
			for (int i = 0; i < nEvents; i++)
			{
				if (pEvents[i]->m_bAutoReset)
				{
					pEvents[i]->m_bSignaled.store(false, std::memory_order::memory_order_release);
				}
			}
		}
		else if (timeout != 0) // Do it one more time under a lock to make sure.
		{
			auto waitForEventsAll = [&bRet, timeout, nEvents, &pEvents, lPredSignaledAll](auto& lock)
			{
				// If we're already signaled, skip adding listeners
				if (lPredSignaledAll())
				{
					bRet = true;
				}
				else if (timeout != 0)
				{
					std::shared_ptr<std::condition_variable_any> condition = std::make_shared<std::condition_variable_any>();

					// We don't have a lock while waiting, so if something gets signaled then unsignaled, we have to add back our listener, because it got popped from the signal, but we fail our condition after leaving the wait state
					do
					{
						// Add listeners
						for (int i = 0; i < nEvents; i++)
						{
							pEvents[i]->AddListenerNoLock(condition);
						}
						if (timeout == TT_INFINITE)
						{
							condition->wait(lock);
							bRet = true;
						}
						else
						{
							bRet = condition->wait_for(lock, std::chrono::milliseconds(timeout)) == std::cv_status::no_timeout;
						}
						// Clear out listeners
						for (int i = 0; i < nEvents; i++)
						{
							pEvents[i]->RemoveListenerNoLock(condition);
						}
					} while (bRet && !lPredSignaledAll());

					condition.reset();
				}

				// Auto reset, since this function is a wait too!
				if (bRet)
				{
					for (int i = 0; i < nEvents; i++)
					{
						if (pEvents[i]->m_bAutoReset)
						{
							pEvents[i]->m_bSignaled.store(false, std::memory_order::memory_order_release);
						}
					}
				}
			};

			switch (nEvents)
			{
				case 1:
				{
					std::unique_lock lock(pEvents[0]->m_Mutex);
					waitForEventsAll(lock);
					break;
				}
				case 2:
				{
					CExtendedScopedLock lock(pEvents[0]->m_Mutex, pEvents[1]->m_Mutex);
					waitForEventsAll(lock);
					break;
				}
				case 3:
				{
					CExtendedScopedLock lock(pEvents[0]->m_Mutex, pEvents[1]->m_Mutex, pEvents[2]->m_Mutex);
					waitForEventsAll(lock);
					break;
				}
				case 4:
				{
					CExtendedScopedLock lock(pEvents[0]->m_Mutex, pEvents[1]->m_Mutex, pEvents[2]->m_Mutex, pEvents[3]->m_Mutex);
					waitForEventsAll(lock);
					break;
				}
				case 5:
				{
					CExtendedScopedLock lock(pEvents[0]->m_Mutex, pEvents[1]->m_Mutex, pEvents[2]->m_Mutex, pEvents[3]->m_Mutex, pEvents[4]->m_Mutex);
					waitForEventsAll(lock);
					break;
				}
				case 6:
				{
					CExtendedScopedLock lock(pEvents[0]->m_Mutex, pEvents[1]->m_Mutex, pEvents[2]->m_Mutex, pEvents[3]->m_Mutex, pEvents[4]->m_Mutex, pEvents[5]->m_Mutex);
					waitForEventsAll(lock);
					break;
				}
				case 7:
				{
					CExtendedScopedLock lock(pEvents[0]->m_Mutex, pEvents[1]->m_Mutex, pEvents[2]->m_Mutex, pEvents[3]->m_Mutex, pEvents[4]->m_Mutex, pEvents[5]->m_Mutex, pEvents[6]->m_Mutex);
					waitForEventsAll(lock);
					break;
				}
				case 8:
				{
					CExtendedScopedLock lock(pEvents[0]->m_Mutex, pEvents[1]->m_Mutex, pEvents[2]->m_Mutex, pEvents[3]->m_Mutex, pEvents[4]->m_Mutex, pEvents[5]->m_Mutex, pEvents[6]->m_Mutex, pEvents[7]->m_Mutex);
					waitForEventsAll(lock);
					break;
				}
				case 9:
				{
					CExtendedScopedLock lock(pEvents[0]->m_Mutex, pEvents[1]->m_Mutex, pEvents[2]->m_Mutex, pEvents[3]->m_Mutex, pEvents[4]->m_Mutex, pEvents[5]->m_Mutex, pEvents[6]->m_Mutex, pEvents[7]->m_Mutex, pEvents[8]->m_Mutex);
					waitForEventsAll(lock);
					break;
				}
				case 10:
				{
					CExtendedScopedLock lock(pEvents[0]->m_Mutex, pEvents[1]->m_Mutex, pEvents[2]->m_Mutex, pEvents[3]->m_Mutex, pEvents[4]->m_Mutex, pEvents[5]->m_Mutex, pEvents[6]->m_Mutex, pEvents[7]->m_Mutex, pEvents[8]->m_Mutex, pEvents[9]->m_Mutex);
					waitForEventsAll(lock);
					break;
				}
				case 11:
				{
					CExtendedScopedLock lock(pEvents[0]->m_Mutex, pEvents[1]->m_Mutex, pEvents[2]->m_Mutex, pEvents[3]->m_Mutex, pEvents[4]->m_Mutex, pEvents[5]->m_Mutex, pEvents[6]->m_Mutex, pEvents[7]->m_Mutex, pEvents[8]->m_Mutex, pEvents[9]->m_Mutex, pEvents[10]->m_Mutex);
					waitForEventsAll(lock);
					break;
				}
				case 12:
				{
					CExtendedScopedLock lock(pEvents[0]->m_Mutex, pEvents[1]->m_Mutex, pEvents[2]->m_Mutex, pEvents[3]->m_Mutex, pEvents[4]->m_Mutex, pEvents[5]->m_Mutex, pEvents[6]->m_Mutex, pEvents[7]->m_Mutex, pEvents[8]->m_Mutex, pEvents[9]->m_Mutex, pEvents[10]->m_Mutex, pEvents[11]->m_Mutex);
					waitForEventsAll(lock);
					break;
				}
				case 13:
				{
					CExtendedScopedLock lock(pEvents[0]->m_Mutex, pEvents[1]->m_Mutex, pEvents[2]->m_Mutex, pEvents[3]->m_Mutex, pEvents[4]->m_Mutex, pEvents[5]->m_Mutex, pEvents[6]->m_Mutex, pEvents[7]->m_Mutex, pEvents[8]->m_Mutex, pEvents[9]->m_Mutex, pEvents[10]->m_Mutex, pEvents[11]->m_Mutex, pEvents[12]->m_Mutex);
					waitForEventsAll(lock);
					break;
				}
				case 14:
				{
					CExtendedScopedLock lock(pEvents[0]->m_Mutex, pEvents[1]->m_Mutex, pEvents[2]->m_Mutex, pEvents[3]->m_Mutex, pEvents[4]->m_Mutex, pEvents[5]->m_Mutex, pEvents[6]->m_Mutex, pEvents[7]->m_Mutex, pEvents[8]->m_Mutex, pEvents[9]->m_Mutex, pEvents[10]->m_Mutex, pEvents[11]->m_Mutex, pEvents[12]->m_Mutex, pEvents[13]->m_Mutex);
					waitForEventsAll(lock);
					break;
				}
				case 15:
				{
					CExtendedScopedLock lock(pEvents[0]->m_Mutex, pEvents[1]->m_Mutex, pEvents[2]->m_Mutex, pEvents[3]->m_Mutex, pEvents[4]->m_Mutex, pEvents[5]->m_Mutex, pEvents[6]->m_Mutex, pEvents[7]->m_Mutex, pEvents[8]->m_Mutex, pEvents[9]->m_Mutex, pEvents[10]->m_Mutex, pEvents[11]->m_Mutex, pEvents[12]->m_Mutex, pEvents[13]->m_Mutex, pEvents[14]->m_Mutex);
					waitForEventsAll(lock);
					break;
				}
				default:
				{
					Warning("Attempted to wait for too many objects!\n");
					Assert(0);
					break;
				}
			}
		}
	}
	else
	{
		auto lPredSignaledAny = [nEvents, &pEvents, &iEventIndex]
		{
			for (int i = 0; i < nEvents; i++)
			{
				if (pEvents[i]->m_bSignaled.load(std::memory_order::memory_order_consume))
				{
					iEventIndex = i;
					return true;
				}
			}

			return false;
		};
		// Optimistic case: we can do an initial check to minimize contention
		if (lPredSignaledAny())
		{
			bRet = true;
			for (int i = 0; i < nEvents; i++)
			{
				if (pEvents[i]->m_bAutoReset)
				{
					pEvents[i]->m_bSignaled.store(false, std::memory_order::memory_order_release);
				}
			}
		}
		else if (timeout != 0) // Do it one more time under a lock to make sure.
		{
			auto waitForEventsAny = [&bRet, timeout, nEvents, &pEvents, lPredSignaledAny, &iEventIndex](auto& lock)
			{
				// If we're already signaled, skip adding listeners
				if (lPredSignaledAny())
				{
					bRet = true;
				}
				else if (timeout != 0)
				{
					std::shared_ptr<std::condition_variable_any> condition = std::make_shared<std::condition_variable_any>();

					// We don't have a lock while waiting, so if something gets signaled then unsignaled, we have to add back our listener, because it got popped from the signal, but we fail our condition after leaving the wait state
					do
					{
						// Add listeners
						for (int i = 0; i < nEvents; i++)
						{
							pEvents[i]->AddListenerNoLock(condition);
						}
						if (timeout == TT_INFINITE)
						{
							condition->wait(lock);
							bRet = true;
						}
						else
						{
							bRet = condition->wait_for(lock, std::chrono::milliseconds(timeout)) == std::cv_status::no_timeout;
						}
						// Clear out listeners
						for (int i = 0; i < nEvents; i++)
						{
							pEvents[i]->RemoveListenerNoLock(condition);
						}
					} while (bRet && !lPredSignaledAny());

					condition.reset();
				}

				// Auto reset, since this function is a wait too!
				if (bRet)
				{
					if (pEvents[iEventIndex]->m_bAutoReset)
					{
						pEvents[iEventIndex]->m_bSignaled.store(false, std::memory_order::memory_order_release);
					}
				}
			};

			// Lock all at the same time, to prevent race conditions.
			// Before, this was implemented by locking and checking for each one after the other, which caused a race condition.
			switch (nEvents)
			{
				case 1:
				{
					std::unique_lock lock(pEvents[0]->m_Mutex);
					waitForEventsAny(lock);
					break;
				}
				case 2:
				{
					CExtendedScopedLock lock(pEvents[0]->m_Mutex, pEvents[1]->m_Mutex);
					waitForEventsAny(lock);
					break;
				}
				case 3:
				{
					CExtendedScopedLock lock(pEvents[0]->m_Mutex, pEvents[1]->m_Mutex, pEvents[2]->m_Mutex);
					waitForEventsAny(lock);
					break;
				}
				case 4:
				{
					CExtendedScopedLock lock(pEvents[0]->m_Mutex, pEvents[1]->m_Mutex, pEvents[2]->m_Mutex, pEvents[3]->m_Mutex);
					waitForEventsAny(lock);
					break;
				}
				case 5:
				{
					CExtendedScopedLock lock(pEvents[0]->m_Mutex, pEvents[1]->m_Mutex, pEvents[2]->m_Mutex, pEvents[3]->m_Mutex, pEvents[4]->m_Mutex);
					waitForEventsAny(lock);
					break;
				}
				case 6:
				{
					CExtendedScopedLock lock(pEvents[0]->m_Mutex, pEvents[1]->m_Mutex, pEvents[2]->m_Mutex, pEvents[3]->m_Mutex, pEvents[4]->m_Mutex, pEvents[5]->m_Mutex);
					waitForEventsAny(lock);
					break;
				}
				case 7:
				{
					CExtendedScopedLock lock(pEvents[0]->m_Mutex, pEvents[1]->m_Mutex, pEvents[2]->m_Mutex, pEvents[3]->m_Mutex, pEvents[4]->m_Mutex, pEvents[5]->m_Mutex, pEvents[6]->m_Mutex);
					waitForEventsAny(lock);
					break;
				}
				case 8:
				{
					CExtendedScopedLock lock(pEvents[0]->m_Mutex, pEvents[1]->m_Mutex, pEvents[2]->m_Mutex, pEvents[3]->m_Mutex, pEvents[4]->m_Mutex, pEvents[5]->m_Mutex, pEvents[6]->m_Mutex, pEvents[7]->m_Mutex);
					waitForEventsAny(lock);
					break;
				}
				case 9:
				{
					CExtendedScopedLock lock(pEvents[0]->m_Mutex, pEvents[1]->m_Mutex, pEvents[2]->m_Mutex, pEvents[3]->m_Mutex, pEvents[4]->m_Mutex, pEvents[5]->m_Mutex, pEvents[6]->m_Mutex, pEvents[7]->m_Mutex, pEvents[8]->m_Mutex);
					waitForEventsAny(lock);
					break;
				}
				case 10:
				{
					CExtendedScopedLock lock(pEvents[0]->m_Mutex, pEvents[1]->m_Mutex, pEvents[2]->m_Mutex, pEvents[3]->m_Mutex, pEvents[4]->m_Mutex, pEvents[5]->m_Mutex, pEvents[6]->m_Mutex, pEvents[7]->m_Mutex, pEvents[8]->m_Mutex, pEvents[9]->m_Mutex);
					waitForEventsAny(lock);
					break;
				}
				case 11:
				{
					CExtendedScopedLock lock(pEvents[0]->m_Mutex, pEvents[1]->m_Mutex, pEvents[2]->m_Mutex, pEvents[3]->m_Mutex, pEvents[4]->m_Mutex, pEvents[5]->m_Mutex, pEvents[6]->m_Mutex, pEvents[7]->m_Mutex, pEvents[8]->m_Mutex, pEvents[9]->m_Mutex, pEvents[10]->m_Mutex);
					waitForEventsAny(lock);
					break;
				}
				case 12:
				{
					CExtendedScopedLock lock(pEvents[0]->m_Mutex, pEvents[1]->m_Mutex, pEvents[2]->m_Mutex, pEvents[3]->m_Mutex, pEvents[4]->m_Mutex, pEvents[5]->m_Mutex, pEvents[6]->m_Mutex, pEvents[7]->m_Mutex, pEvents[8]->m_Mutex, pEvents[9]->m_Mutex, pEvents[10]->m_Mutex, pEvents[11]->m_Mutex);
					waitForEventsAny(lock);
					break;
				}
				case 13:
				{
					CExtendedScopedLock lock(pEvents[0]->m_Mutex, pEvents[1]->m_Mutex, pEvents[2]->m_Mutex, pEvents[3]->m_Mutex, pEvents[4]->m_Mutex, pEvents[5]->m_Mutex, pEvents[6]->m_Mutex, pEvents[7]->m_Mutex, pEvents[8]->m_Mutex, pEvents[9]->m_Mutex, pEvents[10]->m_Mutex, pEvents[11]->m_Mutex, pEvents[12]->m_Mutex);
					waitForEventsAny(lock);
					break;
				}
				case 14:
				{
					CExtendedScopedLock lock(pEvents[0]->m_Mutex, pEvents[1]->m_Mutex, pEvents[2]->m_Mutex, pEvents[3]->m_Mutex, pEvents[4]->m_Mutex, pEvents[5]->m_Mutex, pEvents[6]->m_Mutex, pEvents[7]->m_Mutex, pEvents[8]->m_Mutex, pEvents[9]->m_Mutex, pEvents[10]->m_Mutex, pEvents[11]->m_Mutex, pEvents[12]->m_Mutex, pEvents[13]->m_Mutex);
					waitForEventsAny(lock);
					break;
				}
				case 15:
				{
					CExtendedScopedLock lock(pEvents[0]->m_Mutex, pEvents[1]->m_Mutex, pEvents[2]->m_Mutex, pEvents[3]->m_Mutex, pEvents[4]->m_Mutex, pEvents[5]->m_Mutex, pEvents[6]->m_Mutex, pEvents[7]->m_Mutex, pEvents[8]->m_Mutex, pEvents[9]->m_Mutex, pEvents[10]->m_Mutex, pEvents[11]->m_Mutex, pEvents[12]->m_Mutex, pEvents[13]->m_Mutex, pEvents[14]->m_Mutex);
					waitForEventsAny(lock);
					break;
				}
				default:
				{
					Warning("Attempted to wait for too many objects!\n");
					Assert(0);
					break;
				}
			}
		}
	}
	if (bRet)
	{
		return WAIT_OBJECT_0 + iEventIndex;
	}
	return TW_TIMEOUT;
}


void CStdThreadEvent::AddListener(std::shared_ptr<std::condition_variable_any>& condition)
{
	// Lock because we want to sync m_listeningConditions
	std::scoped_lock lock(m_Mutex);
	AddListenerNoLock(condition);
}

void CStdThreadEvent::AddListenerNoLock(std::shared_ptr<std::condition_variable_any>& condition)
{
    m_listeningConditions.PushItem(condition);
}

void CStdThreadEvent::RemoveListener(std::shared_ptr<std::condition_variable_any>& condition)
{
	// Lock because we want to sync m_listeningConditions
	std::scoped_lock lock(m_Mutex);
	RemoveListenerNoLock(condition);
}

void CStdThreadEvent::RemoveListenerNoLock(std::shared_ptr<std::condition_variable_any>& condition)
{
	m_listeningConditions.RemoveItem(condition);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

CThreadLocalBase::CThreadLocalBase()
{
#ifdef _WIN32
	m_index = TlsAlloc();
	AssertMsg( m_index != 0xFFFFFFFF, "Bad thread local" );
	if ( m_index == 0xFFFFFFFF )
		Error( "Out of thread local storage!\n" );
#elif defined(POSIX)
	if ( pthread_key_create( &m_index, NULL ) != 0 )
		Error( "Out of thread local storage!\n" );
#endif
}

//---------------------------------------------------------

CThreadLocalBase::~CThreadLocalBase()
{
#ifdef _WIN32
	if ( m_index != 0xFFFFFFFF )
		TlsFree( m_index );
	m_index = 0xFFFFFFFF;
#elif defined(POSIX)
	pthread_key_delete( m_index );
#endif
}

//---------------------------------------------------------

void * CThreadLocalBase::Get() const
{
#ifdef _WIN32
	if ( m_index != 0xFFFFFFFF )
		return TlsGetValue( m_index );
	AssertMsg( 0, "Bad thread local" );
	return NULL;
#elif defined(POSIX)
	void *value = pthread_getspecific( m_index );
	return value;
#endif
}

//---------------------------------------------------------

void CThreadLocalBase::Set( void *value )
{
#ifdef _WIN32
	if (m_index != 0xFFFFFFFF)
		TlsSetValue(m_index, value);
	else
		AssertMsg( 0, "Bad thread local" );
#elif defined(POSIX)
	if ( pthread_setspecific( m_index, value ) != 0 )
		AssertMsg( 0, "Bad thread local" );
#endif
}

//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------

#ifdef _WIN32
#ifdef _X360
#define TO_INTERLOCK_PARAM(p)		((long *)p)
#define TO_INTERLOCK_PTR_PARAM(p)	((void **)p)
#else
#define TO_INTERLOCK_PARAM(p)		(p)
#define TO_INTERLOCK_PTR_PARAM(p)	(p)
#endif

#ifndef USE_INTRINSIC_INTERLOCKED
long ThreadInterlockedIncrement( long volatile *pDest )
{
	Assert( (size_t)pDest % 4 == 0 );
	return InterlockedIncrement( TO_INTERLOCK_PARAM(pDest) );
}

long ThreadInterlockedDecrement( long volatile *pDest )
{
	Assert( (size_t)pDest % 4 == 0 );
	return InterlockedDecrement( TO_INTERLOCK_PARAM(pDest) );
}

long ThreadInterlockedExchange( long volatile *pDest, long value )
{
	Assert( (size_t)pDest % 4 == 0 );
	return InterlockedExchange( TO_INTERLOCK_PARAM(pDest), value );
}

long ThreadInterlockedExchangeAdd( long volatile *pDest, long value )
{
	Assert( (size_t)pDest % 4 == 0 );
	return InterlockedExchangeAdd( TO_INTERLOCK_PARAM(pDest), value );
}

long ThreadInterlockedCompareExchange( long volatile *pDest, long value, long comperand )
{
	Assert( (size_t)pDest % 4 == 0 );
	return InterlockedCompareExchange( TO_INTERLOCK_PARAM(pDest), value, comperand );
}

bool ThreadInterlockedAssignIf( long volatile *pDest, long value, long comperand )
{
	Assert( (size_t)pDest % 4 == 0 );

#if !(defined(_WIN64) || defined (_X360))
	__asm 
	{
		mov	eax,comperand
		mov	ecx,pDest
		mov edx,value
		lock cmpxchg [ecx],edx 
		mov eax,0
		setz al
	}
#else
	return ( InterlockedCompareExchange( TO_INTERLOCK_PARAM(pDest), value, comperand ) == comperand );
#endif
}

#endif

#if !defined( USE_INTRINSIC_INTERLOCKED ) || defined( _WIN64 )
void *ThreadInterlockedExchangePointer( void * volatile *pDest, void *value )
{
	Assert( (size_t)pDest % 4 == 0 );
	return InterlockedExchangePointer( TO_INTERLOCK_PARAM(pDest), value );
}

void *ThreadInterlockedCompareExchangePointer( void * volatile *pDest, void *value, void *comperand )
{
	Assert( (size_t)pDest % 4 == 0 );
	return InterlockedCompareExchangePointer( TO_INTERLOCK_PTR_PARAM(pDest), value, comperand );
}

bool ThreadInterlockedAssignPointerIf( void * volatile *pDest, void *value, void *comperand )
{
	Assert( (size_t)pDest % 4 == 0 );
#if !(defined(_WIN64) || defined (_X360))
	__asm 
	{
		mov	eax,comperand
		mov	ecx,pDest
		mov edx,value
		lock cmpxchg [ecx],edx 
		mov eax,0
		setz al
	}
#else
	return ( InterlockedCompareExchangePointer( TO_INTERLOCK_PTR_PARAM(pDest), value, comperand ) == comperand );
#endif
}
#endif

int64 ThreadInterlockedCompareExchange64( int64 volatile *pDest, int64 value, int64 comperand )
{
	Assert( (size_t)pDest % 8 == 0 );

	return InterlockedCompareExchange64( pDest, value, comperand );
}

bool ThreadInterlockedAssignIf64(volatile int64 *pDest, int64 value, int64 comperand ) 
{
	Assert( (size_t)pDest % 8 == 0 );

	return ( ThreadInterlockedCompareExchange64( pDest, value, comperand ) == comperand ); 
}

#if defined( PLATFORM_64BITS )

#if _MSC_VER < 1500
// This intrinsic isn't supported on VS2005.
extern "C" unsigned char _InterlockedCompareExchange128( int64 volatile * Destination, int64 ExchangeHigh, int64 ExchangeLow, int64 * ComparandResult );
#endif

bool ThreadInterlockedAssignIf128( volatile int128 *pDest, const int128 &value, const int128 &comperand )
{
	Assert( ( (size_t)pDest % 16 ) == 0 );

	volatile int64 *pDest64 = ( volatile int64 * )pDest;
	int64 *pValue64 = ( int64 * )&value;
	int64 *pComperand64 = ( int64 * )&comperand;

	// Description:
	//  The CMPXCHG16B instruction compares the 128-bit value in the RDX:RAX and RCX:RBX registers
	//  with a 128-bit memory location. If the values are equal, the zero flag (ZF) is set,
	//  and the RCX:RBX value is copied to the memory location.
	//  Otherwise, the ZF flag is cleared, and the memory value is copied to RDX:RAX.

	// _InterlockedCompareExchange128: http://msdn.microsoft.com/en-us/library/bb514094.aspx
	return _InterlockedCompareExchange128( pDest64, pValue64[1], pValue64[0], pComperand64 ) == 1;
}

#endif // PLATFORM_64BITS

int64 ThreadInterlockedIncrement64( int64 volatile *pDest )
{
	Assert( (size_t)pDest % 8 == 0 );

	int64 Old;

	do 
	{
		Old = *pDest;
	} while (ThreadInterlockedCompareExchange64(pDest, Old + 1, Old) != Old);

	return Old + 1;
}

int64 ThreadInterlockedDecrement64( int64 volatile *pDest )
{
	Assert( (size_t)pDest % 8 == 0 );
	int64 Old;

	do 
	{
		Old = *pDest;
	} while (ThreadInterlockedCompareExchange64(pDest, Old - 1, Old) != Old);

	return Old - 1;
}

int64 ThreadInterlockedExchange64( int64 volatile *pDest, int64 value )
{
	Assert( (size_t)pDest % 8 == 0 );
	int64 Old;

	do 
	{
		Old = *pDest;
	} while (ThreadInterlockedCompareExchange64(pDest, value, Old) != Old);

	return Old;
}

int64 ThreadInterlockedExchangeAdd64( int64 volatile *pDest, int64 value )
{
	Assert( (size_t)pDest % 8 == 0 );
	int64 Old;

	do 
	{
		Old = *pDest;
	} while (ThreadInterlockedCompareExchange64(pDest, Old + value, Old) != Old);

	return Old;
}

#elif defined(GNUC)

#ifdef OSX
#include <libkern/OSAtomic.h>
#endif


long ThreadInterlockedIncrement( long volatile *pDest )
{
	return __sync_fetch_and_add( pDest, 1 ) + 1;
}

long ThreadInterlockedDecrement( long volatile *pDest )
{
	return __sync_fetch_and_sub( pDest, 1 ) - 1;
}

long ThreadInterlockedExchange( long volatile *pDest, long value )
{
	return __sync_lock_test_and_set( pDest, value );
}

long ThreadInterlockedExchangeAdd( long volatile *pDest, long value )
{
	return  __sync_fetch_and_add( pDest, value );
}

long ThreadInterlockedCompareExchange( long volatile *pDest, long value, long comperand )
{
	return  __sync_val_compare_and_swap( pDest, comperand, value );
}

bool ThreadInterlockedAssignIf( long volatile *pDest, long value, long comperand )
{
	return __sync_bool_compare_and_swap( pDest, comperand, value );
}

void *ThreadInterlockedExchangePointer( void * volatile *pDest, void *value )
{
	return __sync_lock_test_and_set( pDest, value );
}

void *ThreadInterlockedCompareExchangePointer( void *volatile *pDest, void *value, void *comperand )
{	
	return  __sync_val_compare_and_swap( pDest, comperand, value );
}

bool ThreadInterlockedAssignPointerIf( void * volatile *pDest, void *value, void *comperand )
{
	return  __sync_bool_compare_and_swap( pDest, comperand, value );
}

int64 ThreadInterlockedCompareExchange64( int64 volatile *pDest, int64 value, int64 comperand )
{
#if defined(OSX)
	int64 retVal = *pDest;
	if ( OSAtomicCompareAndSwap64( comperand, value, pDest ) )
		retVal = *pDest;
	
	return retVal;
#else
	return __sync_val_compare_and_swap( pDest, comperand, value  );
#endif
}

bool ThreadInterlockedAssignIf64( int64 volatile * pDest, int64 value, int64 comperand ) 
{
	return __sync_bool_compare_and_swap( pDest, comperand, value );
}

int64 ThreadInterlockedExchange64( int64 volatile *pDest, int64 value )
{
	Assert( (size_t)pDest % 8 == 0 );
	int64 Old;
	
	do 
	{
		Old = *pDest;
	} while (ThreadInterlockedCompareExchange64(pDest, value, Old) != Old);
	
	return Old;
}


#else
// This will perform horribly,
#error "Falling back to mutexed interlocked operations, you really don't have intrinsics you can use?"ß
CThreadMutex g_InterlockedMutex;

long ThreadInterlockedIncrement( long volatile *pDest )
{
	AUTO_LOCK( g_InterlockedMutex );
	return ++(*pDest);
}

long ThreadInterlockedDecrement( long volatile *pDest )
{
	AUTO_LOCK( g_InterlockedMutex );
	return --(*pDest);
}

long ThreadInterlockedExchange( long volatile *pDest, long value )
{
	AUTO_LOCK( g_InterlockedMutex );
	long retVal = *pDest;
	*pDest = value;
	return retVal;
}

void *ThreadInterlockedExchangePointer( void * volatile *pDest, void *value )
{
	AUTO_LOCK( g_InterlockedMutex );
	void *retVal = *pDest;
	*pDest = value;
	return retVal;
}

long ThreadInterlockedExchangeAdd( long volatile *pDest, long value )
{
	AUTO_LOCK( g_InterlockedMutex );
	long retVal = *pDest;
	*pDest += value;
	return retVal;
}

long ThreadInterlockedCompareExchange( long volatile *pDest, long value, long comperand )
{
	AUTO_LOCK( g_InterlockedMutex );
	long retVal = *pDest;
	if ( *pDest == comperand )
		*pDest = value;
	return retVal;
}

void *ThreadInterlockedCompareExchangePointer( void * volatile *pDest, void *value, void *comperand )
{
	AUTO_LOCK( g_InterlockedMutex );
	void *retVal = *pDest;
	if ( *pDest == comperand )
		*pDest = value;
	return retVal;
}


int64 ThreadInterlockedCompareExchange64( int64 volatile *pDest, int64 value, int64 comperand )
{
	Assert( (size_t)pDest % 8 == 0 );
	AUTO_LOCK( g_InterlockedMutex );
	int64 retVal = *pDest;
	if ( *pDest == comperand )
		*pDest = value;
	return retVal;
}

int64 ThreadInterlockedExchange64( int64 volatile *pDest, int64 value )
{
	Assert( (size_t)pDest % 8 == 0 );
	int64 Old;

	do 
	{
		Old = *pDest;
	} while (ThreadInterlockedCompareExchange64(pDest, value, Old) != Old);

	return Old;
}

bool ThreadInterlockedAssignIf64(volatile int64 *pDest, int64 value, int64 comperand ) 
{
	Assert( (size_t)pDest % 8 == 0 );
	return ( ThreadInterlockedCompareExchange64( pDest, value, comperand ) == comperand ); 
}

bool ThreadInterlockedAssignIf( long volatile *pDest, long value, long comperand )
{
	Assert( (size_t)pDest % 4 == 0 );
	return ( ThreadInterlockedCompareExchange( pDest, value, comperand ) == comperand ); 
}

#endif

//-----------------------------------------------------------------------------

#if defined(_WIN32) && defined(THREAD_PROFILER)
void ThreadNotifySyncNoop(void *p) {}

#define MAP_THREAD_PROFILER_CALL( from, to ) \
	void from(void *p) \
	{ \
		static CDynamicFunction<void (*)(void *)> dynFunc( "libittnotify.dll", #to, ThreadNotifySyncNoop ); \
		(*dynFunc)(p); \
	}

MAP_THREAD_PROFILER_CALL( ThreadNotifySyncPrepare, __itt_notify_sync_prepare );
MAP_THREAD_PROFILER_CALL( ThreadNotifySyncCancel, __itt_notify_sync_cancel );
MAP_THREAD_PROFILER_CALL( ThreadNotifySyncAcquired, __itt_notify_sync_acquired );
MAP_THREAD_PROFILER_CALL( ThreadNotifySyncReleasing, __itt_notify_sync_releasing );

#endif

//-----------------------------------------------------------------------------
//
// CThreadMutex
//
//-----------------------------------------------------------------------------

#ifndef POSIX
CThreadMutex::CThreadMutex()
{
#ifdef THREAD_MUTEX_TRACING_ENABLED
	memset( &m_CriticalSection, 0, sizeof(m_CriticalSection) );
#endif
	InitializeCriticalSectionAndSpinCount((CRITICAL_SECTION *)&m_CriticalSection, 20);
#ifdef THREAD_MUTEX_TRACING_SUPPORTED
	// These need to be initialized unconditionally in case mixing release & debug object modules
	// Lock and unlock may be emitted as COMDATs, in which case may get spurious output
	m_currentOwnerID = m_lockCount = 0;
	m_bTrace = false;
#endif
}

CThreadMutex::~CThreadMutex()
{
	DeleteCriticalSection((CRITICAL_SECTION *)&m_CriticalSection);
}
#endif // !POSIX

#if defined( _WIN32 ) && !defined( _X360 )
typedef BOOL (WINAPI*TryEnterCriticalSectionFunc_t)(LPCRITICAL_SECTION);
static CDynamicFunction<TryEnterCriticalSectionFunc_t> DynTryEnterCriticalSection( "Kernel32.dll", "TryEnterCriticalSection" );
#elif defined( _X360 )
#define DynTryEnterCriticalSection TryEnterCriticalSection
#endif

bool CThreadMutex::TryLock()
{

#if defined( _WIN32 )
#ifdef THREAD_MUTEX_TRACING_ENABLED
	uint thisThreadID = ThreadGetCurrentId();
	if ( m_bTrace && m_currentOwnerID && ( m_currentOwnerID != thisThreadID ) )
		Msg( "Thread %u about to try-wait for lock %p owned by %u\n", ThreadGetCurrentId(), (CRITICAL_SECTION *)&m_CriticalSection, m_currentOwnerID );
#endif
	if ( DynTryEnterCriticalSection != NULL )
	{
		if ( (*DynTryEnterCriticalSection )( (CRITICAL_SECTION *)&m_CriticalSection ) != FALSE )
		{
#ifdef THREAD_MUTEX_TRACING_ENABLED
			if (m_lockCount == 0)
			{
				// we now own it for the first time.  Set owner information
				m_currentOwnerID = thisThreadID;
				if ( m_bTrace )
					Msg( "Thread %u now owns lock 0x%p\n", m_currentOwnerID, (CRITICAL_SECTION *)&m_CriticalSection );
			}
			m_lockCount++;
#endif
			return true;
		}
		return false;
	}
	Lock();
	return true;
#elif defined( POSIX )
	 return pthread_mutex_trylock( &m_Mutex ) == 0;
#else
#error "Implement me!"
	return true;
#endif
}

//-----------------------------------------------------------------------------
//
// CThreadFastMutex
//
//-----------------------------------------------------------------------------

#define THREAD_SPIN (1024)

void CThreadFastMutex::Lock( const uint32 threadId, unsigned nSpinSleepTime ) volatile 
{
	int i;
	if ( nSpinSleepTime != TT_INFINITE )
	{
		for ( i = THREAD_SPIN * g_yieldsPerNormalizedYield; i != 0; --i )
		{
			if ( TryLock( threadId ) )
			{
				return;
			}
			ThreadPause();
		}

		for ( i = THREAD_SPIN * g_yieldsPerNormalizedYield; i != 0; --i )
		{
			if ( TryLock( threadId ) )
			{
				return;
			}
			ThreadPause();
			if ( i % 1024 == 0 )
			{
				ThreadSleep( 0 );
			}
		}

#ifdef _WIN32
		if ( !nSpinSleepTime && GetThreadPriority( GetCurrentThread() ) > THREAD_PRIORITY_NORMAL )
		{
			nSpinSleepTime = 1;
		} 
		else
#endif

		if ( nSpinSleepTime )
		{
			for ( i = THREAD_SPIN * g_yieldsPerNormalizedYield; i != 0; --i )
			{
				if ( TryLock( threadId ) )
				{
					return;
				}

				ThreadPause();
				ThreadSleep( 0 );
			}

		}

		for ( ;; ) // coded as for instead of while to make easy to breakpoint success
		{
			if ( TryLock( threadId ) )
			{
				return;
			}

			ThreadPause();
			ThreadSleep( nSpinSleepTime );
		}
	}
	else
	{
		for ( ;; ) // coded as for instead of while to make easy to breakpoint success
		{
			if ( TryLock( threadId ) )
			{
				return;
			}

			ThreadPause();
		}
	}
}

#ifdef WIN32
void CThreadSpinningMutex::Unlock()
{
	::ReleaseSRWLockExclusive(reinterpret_cast<PSRWLOCK>(&lock_));
}

bool CThreadSpinningMutex::TryLock()
{
	return !!::TryAcquireSRWLockExclusive(reinterpret_cast<PSRWLOCK>(&lock_));
}

void CThreadSpinningMutex::LockSlow()
{
	::AcquireSRWLockExclusive(reinterpret_cast<PSRWLOCK>(&lock_));
}
#endif

//-----------------------------------------------------------------------------
//
// CThreadRWLock
//
//-----------------------------------------------------------------------------

void CThreadRWLock::WaitForRead()
{
	m_nPendingReaders++;

	do
	{
		m_mutex.Unlock();
		m_CanRead.Wait();
		m_mutex.Lock();
	}
	while (m_nWriters);

	m_nPendingReaders--;
}


void CThreadRWLock::LockForWrite()
{
	m_mutex.Lock();
	bool bWait = ( m_nWriters != 0 || m_nActiveReaders != 0 );
	m_nWriters++;
	m_CanRead.Reset();
	m_mutex.Unlock();

	if ( bWait )
	{
		m_CanWrite.Wait();
	}
}

void CThreadRWLock::UnlockWrite()
{
	m_mutex.Lock();
	m_nWriters--;
	if ( m_nWriters == 0)
	{
		if ( m_nPendingReaders )
		{
			m_CanRead.Set();
		}
	}
	else
	{
		m_CanWrite.Set();
	}
	m_mutex.Unlock();
}

//-----------------------------------------------------------------------------
//
// CThreadSpinRWLock
//
//-----------------------------------------------------------------------------

void CThreadSpinRWLock::SpinLockForWrite( const uint32 threadId )
{
	int i;

	for ( i = 128 * g_yieldsPerNormalizedYield; i != 0; --i )
	{
		if ( TryLockForWrite( threadId ) )
		{
			return;
		}
		ThreadPause();
	}

	for ( i = 2560 * g_yieldsPerNormalizedYield; i != 0; --i )
	{
		if ( TryLockForWrite( threadId ) )
		{
			return;
		}

		ThreadPause();
		ThreadSleep( 0 );
	}

	for ( ;; ) // coded as for instead of while to make easy to breakpoint success
	{
		if ( TryLockForWrite( threadId ) )
		{
			return;
		}

		ThreadPause();
		ThreadSleep( 1 );
	}
}

void CThreadSpinRWLock::LockForRead()
{
	int i;

	// In order to grab a read lock, the number of readers must not change and no thread can own the write lock
	LockInfo_t oldValue;
	LockInfo_t newValue;

	oldValue.m_nReaders = m_lockInfo.m_nReaders;
	oldValue.m_writerId = 0;
	newValue.m_nReaders = oldValue.m_nReaders + 1;
	newValue.m_writerId = 0;

	if( m_nWriters == 0 && AssignIf( newValue, oldValue ) )
		return;
	ThreadPause();
	oldValue.m_nReaders = m_lockInfo.m_nReaders;
	newValue.m_nReaders = oldValue.m_nReaders + 1;

	for ( i = 128 * g_yieldsPerNormalizedYield; i != 0; --i )
	{
		if( m_nWriters == 0 && AssignIf( newValue, oldValue ) )
			return;
		ThreadPause();
		oldValue.m_nReaders = m_lockInfo.m_nReaders;
		newValue.m_nReaders = oldValue.m_nReaders + 1;
	}

	for ( i = 2560 * g_yieldsPerNormalizedYield; i != 0; --i )
	{
		if( m_nWriters == 0 && AssignIf( newValue, oldValue ) )
			return;
		ThreadPause();
		ThreadSleep( 0 );
		oldValue.m_nReaders = m_lockInfo.m_nReaders;
		newValue.m_nReaders = oldValue.m_nReaders + 1;
	}

	for ( ;; ) // coded as for instead of while to make easy to breakpoint success
	{
		if( m_nWriters == 0 && AssignIf( newValue, oldValue ) )
			return;
		ThreadPause();
		ThreadSleep( 1 );
		oldValue.m_nReaders = m_lockInfo.m_nReaders;
		newValue.m_nReaders = oldValue.m_nReaders + 1;
	}
}

void CThreadSpinRWLock::UnlockRead()
{
	int i;

	Assert( m_lockInfo.m_nReaders > 0 && m_lockInfo.m_writerId == 0 );
	LockInfo_t oldValue;
	LockInfo_t newValue;

	oldValue.m_nReaders = m_lockInfo.m_nReaders;
	oldValue.m_writerId = 0;
	newValue.m_nReaders = oldValue.m_nReaders - 1;
	newValue.m_writerId = 0;

	if( AssignIf( newValue, oldValue ) )
		return;
	ThreadPause();
	oldValue.m_nReaders = m_lockInfo.m_nReaders;
	newValue.m_nReaders = oldValue.m_nReaders - 1;

	for ( i = 500; i != 0; --i )
	{
		if( AssignIf( newValue, oldValue ) )
			return;
		ThreadPause();
		oldValue.m_nReaders = m_lockInfo.m_nReaders;
		newValue.m_nReaders = oldValue.m_nReaders - 1;
	}

	for ( i = 20000; i != 0; --i )
	{
		if( AssignIf( newValue, oldValue ) )
			return;
		ThreadPause();
		ThreadSleep( 0 );
		oldValue.m_nReaders = m_lockInfo.m_nReaders;
		newValue.m_nReaders = oldValue.m_nReaders - 1;
	}

	for ( ;; ) // coded as for instead of while to make easy to breakpoint success
	{
		if( AssignIf( newValue, oldValue ) )
			return;
		ThreadPause();
		ThreadSleep( 1 );
		oldValue.m_nReaders = m_lockInfo.m_nReaders;
		newValue.m_nReaders = oldValue.m_nReaders - 1;
	}
}

void CThreadSpinRWLock::UnlockWrite()
{
	Assert( m_lockInfo.m_writerId == ThreadGetCurrentId()  && m_lockInfo.m_nReaders == 0 );
	static const LockInfo_t newValue = { 0, 0 };
#if defined(_X360)
	// X360TBD: Serious Perf implications, not yet. __sync();
#endif
	ThreadInterlockedExchange64(  (int64 *)&m_lockInfo, *((int64 *)&newValue) );
	m_nWriters--;
}



//-----------------------------------------------------------------------------
//
// CThread
//
//-----------------------------------------------------------------------------

CThreadLocalPtr<CThread> g_pCurThread;

//---------------------------------------------------------

CThread::CThread()
:	
#ifdef _WIN32
	m_hThread( NULL ),
#endif
	m_threadId( 0 ),
	m_result( 0 ),
	m_flags( 0 )
{
	m_szName[0] = 0;
}

//---------------------------------------------------------

CThread::~CThread()
{
#ifdef _WIN32
	if (m_hThread)
#elif defined(POSIX)
	if ( m_threadId )
#endif
	{
		if ( IsAlive() )
		{
			Msg( "Illegal termination of worker thread! Threads must negotiate an end to the thread before the CThread object is destroyed.\n" ); 
#ifdef _WIN32

			DoNewAssertDialog( __FILE__, __LINE__, "Illegal termination of worker thread! Threads must negotiate an end to the thread before the CThread object is destroyed.\n" );
#endif
			if ( GetCurrentCThread() == this )
			{
				Stop(); // BUGBUG: Alfred - this doesn't make sense, this destructor fires from the hosting thread not the thread itself!!
			}
		}

#ifdef _WIN32
		// Now that the worker thread has exited (which we know because we presumably waited
		// on the thread handle for it to exit) we can finally close the thread handle. We
		// cannot do this any earlier, and certainly not in CThread::ThreadProc().
		CloseHandle( m_hThread );
#endif
	}
}


//---------------------------------------------------------

const char *CThread::GetName()
{
	AUTO_LOCK( m_Lock );
	if ( !m_szName[0] )
	{
#ifdef _WIN32
		_snprintf( m_szName, sizeof(m_szName) - 1, "Thread(%p/%p)", this, m_hThread );
#elif defined(POSIX)
		_snprintf( m_szName, sizeof(m_szName) - 1, "Thread(0x%x/0x%x)", (uint)this, (uint)m_threadId );
#endif
		m_szName[sizeof(m_szName) - 1] = 0;
	}
	return m_szName;
}

//---------------------------------------------------------

void CThread::SetName(const char *pszName)
{
	AUTO_LOCK( m_Lock );
	strncpy( m_szName, pszName, sizeof(m_szName) - 1 );
	m_szName[sizeof(m_szName) - 1] = 0;
}

//---------------------------------------------------------

bool CThread::Start( unsigned nBytesStack )
{
	AUTO_LOCK( m_Lock );

	if ( IsAlive() )
	{
		AssertMsg( 0, "Tried to create a thread that has already been created!" );
		return false;
	}

	bool  bInitSuccess = false;
	CStdThreadEvent createComplete;
	ThreadInit_t init = { this, &createComplete, &bInitSuccess };

#ifdef _WIN32
	HANDLE       hThread;
	m_hThread = hThread = (HANDLE)VCRHook_CreateThread( NULL,
														nBytesStack,
														(LPTHREAD_START_ROUTINE)GetThreadProc(),
														new ThreadInit_t(init),
														CREATE_SUSPENDED,
														&m_threadId );
	if ( !hThread )
	{
		AssertMsg1( 0, "Failed to create thread (error 0x%x)", GetLastError() );
		return false;
	}
	Plat_ApplyHardwareDataBreakpointsToNewThread( m_threadId );
	ResumeThread( hThread );

#elif defined(POSIX)
	pthread_attr_t attr;
	pthread_attr_init( &attr );
	// From http://www.kernel.org/doc/man-pages/online/pages/man3/pthread_attr_setstacksize.3.html
	//  A thread's stack size is fixed at the time of thread creation. Only the main thread can dynamically grow its stack.
	pthread_attr_setstacksize( &attr, MAX( nBytesStack, 1024u*1024 ) );
	if ( pthread_create( &m_threadId, &attr, (void *(*)(void *))GetThreadProc(), new ThreadInit_t( init ) ) != 0 )
	{
		AssertMsg1( 0, "Failed to create thread (error 0x%x)", GetLastError() );
		return false;
	}
	Plat_ApplyHardwareDataBreakpointsToNewThread( (long unsigned int)m_threadId );
	bInitSuccess = true;
#endif

#if !defined( OSX )
	ThreadSetDebugName( m_threadId, m_szName );
#endif

	if ( !WaitForCreateComplete( &createComplete ) )
	{
		Msg( "Thread failed to initialize\n" );
#ifdef _WIN32
		CloseHandle( m_hThread );
		m_hThread = NULL;
		m_threadId = 0;
#elif defined(POSIX)
		m_threadId = 0;
#endif
		return false;
	}

	if ( !bInitSuccess )
	{
		Msg( "Thread failed to initialize\n" );
#ifdef _WIN32
		CloseHandle( m_hThread );
		m_hThread = NULL;
		m_threadId = 0;
#elif defined(POSIX)
		m_threadId = 0;
#endif
		return false;
	}

#ifdef _WIN32
	if ( !m_hThread )
	{
		Msg( "Thread exited immediately\n" );
	}
#endif

#ifdef _WIN32
	return !!m_hThread;
#elif defined(POSIX)
	return !!m_threadId;
#endif
}

//---------------------------------------------------------
//
// Return true if the thread exists. false otherwise
//

bool CThread::IsAlive()
{
#ifdef _WIN32
	DWORD dwExitCode;

	return ( m_hThread &&
		GetExitCodeThread( m_hThread, &dwExitCode ) &&
		dwExitCode == STILL_ACTIVE );
#elif defined(POSIX)
	return m_threadId;
#endif
}

//---------------------------------------------------------

bool CThread::Join(unsigned timeout)
{
#ifdef _WIN32
	if ( m_hThread )
#elif defined(POSIX)
	if ( m_threadId )
#endif
	{
		AssertMsg(GetCurrentCThread() != this, _T("Thread cannot be joined with self"));

#ifdef _WIN32
		return ThreadJoin( (ThreadHandle_t)m_hThread );
#elif defined(POSIX)
		return ThreadJoin( (ThreadHandle_t)m_threadId );
#endif
	}
	return true;
}

//---------------------------------------------------------

#ifdef _WIN32

HANDLE CThread::GetThreadHandle()
{
	return m_hThread;
}

#endif

#if defined( _WIN32 ) || defined( LINUX )

//---------------------------------------------------------

uint CThread::GetThreadId()
{
	return m_threadId;
}

#endif

//---------------------------------------------------------

int CThread::GetResult()
{
	return m_result;
}

//---------------------------------------------------------
//
// Forcibly, abnormally, but relatively cleanly stop the thread
//

void CThread::Stop(int exitCode)
{
	if ( !IsAlive() )
		return;

	if ( GetCurrentCThread() == this )
	{
		m_result = exitCode;
		if ( !( m_flags & SUPPORT_STOP_PROTOCOL ) )
		{
			OnExit();
			g_pCurThread = (int)NULL;

#ifdef _WIN32
			CloseHandle( m_hThread );
			m_hThread = NULL;
#endif
			Cleanup();
		}
		throw exitCode;
	}
	else
		AssertMsg( 0, "Only thread can stop self: Use a higher-level protocol");
}

//---------------------------------------------------------

int CThread::GetPriority() const
{
#ifdef _WIN32
	return GetThreadPriority(m_hThread);
#elif defined(POSIX)
	struct sched_param thread_param;
	int policy;
	pthread_getschedparam( m_threadId, &policy, &thread_param );
	return thread_param.sched_priority;
#endif
}

//---------------------------------------------------------

bool CThread::SetPriority(int priority)
{
#ifdef _WIN32
	return ThreadSetPriority( (ThreadHandle_t)m_hThread, priority );
#else
	return ThreadSetPriority( (ThreadHandle_t)m_threadId, priority );
#endif
}


//---------------------------------------------------------

void CThread::SuspendCooperative()
{
	if ( ThreadGetCurrentId() == (ThreadId_t)m_threadId )
	{
		m_SuspendEventSignal.Set();
		m_nSuspendCount = 1;
		m_SuspendEvent.Wait();
		m_nSuspendCount = 0;
	}
	else
	{
		Assert( !"Suspend not called from worker thread, this would be a bug" );
	}
}

//---------------------------------------------------------

void CThread::ResumeCooperative()
{
	Assert( m_nSuspendCount == 1 );
	m_SuspendEvent.Set();
}


void CThread::BWaitForThreadSuspendCooperative()
{
	m_SuspendEventSignal.Wait();
}


#ifndef LINUX
//---------------------------------------------------------

unsigned int CThread::Suspend()
{
#ifdef _WIN32
	return ( SuspendThread(m_hThread) != 0 );
#elif defined(OSX)
	int susCount = m_nSuspendCount++;
	while ( thread_suspend( pthread_mach_thread_np(m_threadId) ) != KERN_SUCCESS )
	{
	};
	return ( susCount) != 0;
#else
#error
#endif
}

//---------------------------------------------------------

unsigned int CThread::Resume()
{
#ifdef _WIN32
	return ( ResumeThread(m_hThread) != 0 );
#elif defined(OSX)
	int susCount = m_nSuspendCount++;
	while ( thread_resume( pthread_mach_thread_np(m_threadId) )  != KERN_SUCCESS )
	{
	};	
	return ( susCount - 1) != 0;
#else
#error
#endif
}
#endif


//---------------------------------------------------------

bool CThread::Terminate(int exitCode)
{
#ifndef _X360
#ifdef _WIN32
	// I hope you know what you're doing!
	if (!TerminateThread(m_hThread, exitCode))
		return false;
	CloseHandle( m_hThread );
	m_hThread = NULL;
	Cleanup();
#elif defined(POSIX)
	pthread_kill( m_threadId, SIGKILL );
	Cleanup();
#endif

	return true;
#else
	AssertMsg( 0, "Cannot terminate a thread on the Xbox!" );
	return false;
#endif
}

//---------------------------------------------------------
//
// Get the Thread object that represents the current thread, if any.
// Can return NULL if the current thread was not created using
// CThread
//

CThread *CThread::GetCurrentCThread()
{
	return g_pCurThread;
}

//---------------------------------------------------------
//
// Offer a context switch. Under Win32, equivalent to Sleep(0)
//

void CThread::Yield()
{
#ifdef _WIN32
	::Sleep(0);
#elif defined(POSIX)
	pthread_yield();
#endif
}

//---------------------------------------------------------
//
// This method causes the current thread to yield and not to be
// scheduled for further execution until a certain amount of real
// time has elapsed, more or less.
//

void CThread::Sleep(unsigned duration)
{
#ifdef _WIN32
	::Sleep(duration);
#elif defined(POSIX)
	usleep( duration * 1000 );
#endif
}

//---------------------------------------------------------

bool CThread::Init()
{
	return true;
}

//---------------------------------------------------------

void CThread::OnExit()
{
}

//---------------------------------------------------------

void CThread::Cleanup()
{
	m_threadId = 0;
}

//---------------------------------------------------------
bool CThread::WaitForCreateComplete(CStdThreadEvent * pEvent)
{
	// Force serialized thread creation...
#ifdef THREADS_DEBUG
	if (!pEvent->Wait(10000))
#else
	if (!pEvent->Wait())
#endif
	{
		AssertMsg( 0, "Probably deadlock or failure waiting for thread to initialize." );
		return false;
	}
	return true;
}

//---------------------------------------------------------

bool CThread::IsThreadRunning()
{
#ifdef _PS3
    // ThreadIsThreadIdRunning() doesn't work on PS3 if the thread is in a zombie state
    return m_eventTheadExit.Check();
#else
    return ThreadIsThreadIdRunning( (ThreadId_t)m_threadId );
#endif
}

//---------------------------------------------------------

CThread::ThreadProc_t CThread::GetThreadProc()
{
	return ThreadProc;
}

//---------------------------------------------------------

unsigned __stdcall CThread::ThreadProc(LPVOID pv)
{
  std::unique_ptr<ThreadInit_t> pInit((ThreadInit_t *)pv);
  
#ifdef _X360
        // Make sure all threads are consistent w.r.t floating-point math
	SetupFPUControlWord();
#endif
	
	CThread *pThread = pInit->pThread;
	g_pCurThread = pThread;
	
	g_pCurThread->m_pStackBase = AlignValue( &pThread, 4096 );
	
	pInit->pThread->m_result = -1;
	
	bool bInitSuccess = true;
	if ( pInit->pfInitSuccess )
		*(pInit->pfInitSuccess) = false;
	
	try
	{
		bInitSuccess = pInit->pThread->Init();
	}
	
	catch (...)
	{
		pInit->pInitCompleteEvent->Set();
		throw;
	}
	
	if ( pInit->pfInitSuccess )
		*(pInit->pfInitSuccess) = bInitSuccess;
	pInit->pInitCompleteEvent->Set();
	if (!bInitSuccess)
		return 0;
	
	if ( pInit->pThread->m_flags & SUPPORT_STOP_PROTOCOL )
	{
		try
		{
			pInit->pThread->m_result = pInit->pThread->Run();
		}
		
		catch (...)
		{
		}
	}
	else
	{
		pInit->pThread->m_result = pInit->pThread->Run();
	}
	
	pInit->pThread->OnExit();
	g_pCurThread = (int)NULL;
	pInit->pThread->Cleanup();
	
	return pInit->pThread->m_result;
}


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
CWorkerThread::CWorkerThread()
:         
	m_Param(0),
	m_pParamFunctor(NULL),
	m_ReturnVal(0)
{
}

//---------------------------------------------------------

int CWorkerThread::CallWorker(unsigned dw, unsigned timeout, bool fBoostWorkerPriorityToMaster, CFunctor *pParamFunctor)
{
	return Call(dw, timeout, fBoostWorkerPriorityToMaster, NULL, pParamFunctor);
}

//---------------------------------------------------------

int CWorkerThread::CallMaster(unsigned dw, unsigned timeout)
{
	return Call(dw, timeout, false);
}

//---------------------------------------------------------

CStdThreadEvent &CWorkerThread::GetCallHandle()
{
	return m_EventSend;
}

//---------------------------------------------------------

unsigned CWorkerThread::GetCallParam( CFunctor **ppParamFunctor ) const
{
	if( ppParamFunctor )
		*ppParamFunctor = m_pParamFunctor;
	return m_Param;
}

//---------------------------------------------------------

int CWorkerThread::BoostPriority()
{
	int iInitialPriority = GetPriority();
	const int iNewPriority = ThreadGetPriority( (ThreadHandle_t)GetThreadID() );
	if (iNewPriority > iInitialPriority)
		ThreadSetPriority( (ThreadHandle_t)GetThreadID(), iNewPriority);
	return iInitialPriority;
}

//---------------------------------------------------------

static uint32 __stdcall DefaultWaitFunc( int nEvents, CStdThreadEvent * const *pEvents, int bWaitAll, uint32 timeout )
{
	return CStdThreadEvent::WaitForMultiple(nEvents, pEvents, bWaitAll != 0, timeout);
}

int CWorkerThread::Call(unsigned dwParam, unsigned timeout, bool fBoostPriority, WaitFunc_t pfnWait, CFunctor *pParamFunctor)
{
	AssertMsg(!m_EventSend.Check(), "Cannot perform call if there's an existing call pending" );

	AUTO_LOCK( m_Lock );

	if (!IsAlive())
		return WTCR_FAIL;

	int iInitialPriority = 0;
	if (fBoostPriority)
	{
		iInitialPriority = BoostPriority();
	}

	// set the parameter, signal the worker thread, wait for the completion to be signaled
	m_Param = dwParam;
	m_pParamFunctor = pParamFunctor;

	m_EventComplete.Reset();
	m_EventSend.Set();

	// UNDONE: we no longer wait for a reply when calling
	// WaitForReply( timeout, pfnWait );

	// MWD: Investigate why setting thread priorities is killing the 360
#ifndef _X360
	if (fBoostPriority)
		SetPriority(iInitialPriority);
#endif

	// We always timeout, because we don't wait.
	return WTCR_TIMEOUT;
}

//---------------------------------------------------------
//
// Wait for a request from the client
//
//---------------------------------------------------------
int CWorkerThread::WaitForReply( unsigned timeout )
{
	return WaitForReply( timeout, NULL );
}

int CWorkerThread::WaitForReply( unsigned timeout, WaitFunc_t pfnWait )
{
	bool result;
#ifdef THREADS_DEBUG
	bool bInDebugger = Plat_IsInDebugSession();

	do
	{
		result = m_EventComplete.Wait(timeout != TT_INFINITE ? timeout : 10000);

		AssertMsg(timeout != TT_INFINITE || result != WAIT_TIMEOUT, "Possible hung thread, call to thread timed out");

	} while (bInDebugger && (timeout == TT_INFINITE && !result));
#else
	result = m_EventComplete.Wait(timeout);
#endif

	if (result)
	{
		m_EventSend.Reset();
	}
	else
	{
		DevMsg(2, "Thread failed to respond, probably exited\n");
		m_ReturnVal = WTCR_TIMEOUT;
	}

	return m_ReturnVal;
}


//---------------------------------------------------------
//
// Wait for a request from the client
//
//---------------------------------------------------------

bool CWorkerThread::WaitForCall(unsigned * pResult)
{
	return WaitForCall(TT_INFINITE, pResult);
}

//---------------------------------------------------------

bool CWorkerThread::WaitForCall(unsigned dwTimeout, unsigned * pResult)
{
	bool returnVal = m_EventSend.Wait(dwTimeout);
	if (pResult)
		*pResult = m_Param;
	return returnVal;
}

//---------------------------------------------------------
//
// is there a request?
//

bool CWorkerThread::PeekCall(unsigned * pParam, CFunctor **ppParamFunctor)
{
	if (!m_EventSend.Check())
	{
		return false;
	}
	else
	{
		if (pParam)
		{
			*pParam = m_Param;
		}
		if( ppParamFunctor )
		{
			*ppParamFunctor = m_pParamFunctor;
		}
		return true;
	}
}

//---------------------------------------------------------
//
// Reply to the request
//

void CWorkerThread::Reply(unsigned dw)
{
	m_Param = 0;
	m_ReturnVal = dw;

	// The request is now complete so PeekCall() should fail from
	// now on
	//
	// This event should be reset BEFORE we signal the client
	m_EventSend.Reset();

	// Tell the client we're finished
	m_EventComplete.Set();
}

//-----------------------------------------------------------------------------
