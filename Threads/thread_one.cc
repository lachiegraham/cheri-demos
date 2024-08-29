#include <cheri.hh>
#include <debug.hh>
#include <errno.h>
#include <thread.h>
#include <fail-simulator-on-error.h>
#include <platform-gpio.hh>



/// Expose debugging features unconditionally for this compartment.
using Debug = ConditionalDebug<true, "Thread 1">;

// Import some useful things from the CHERI namespace.
using namespace CHERI;


// nothing unexpected happens here

void __cheri_compartment("thread_one") entry()
{
    Debug::log("starting thread {}",thread_id_get());

    const char *allocation = "Bananas and apples";
    Debug::log("Writing secret message at {}", &allocation);

    Debug::log("Waiting");
    while (thread_count() > 1) {

    }
    Debug::log("ending thread");
    return;
}