#include <debug.hh>
#include <fail-simulator-on-error.h>
#include <timeout.hh>
#include <token.h>
#include "capability_sender.h"
#include "capability_receiver.h"



/// Expose debugging features unconditionally for this compartment.
using Debug = ConditionalDebug<true, "Sender">;

// Import some useful things from the CHERI namespace.
using namespace CHERI;


void transfer_back(const char* allocation) {

    Debug::log("Transferred back: {}", allocation);
    Debug::log("Capability info: {}", &allocation);


    Debug::log("Fiddling with capability");
    Debug::log("Capability info: {}", *(&allocation + 10));
    // The capability seems to be safe from fiddling
    //  It is put in a read_only state, and even when I force
    //   some kind of error (see above) the sender compartment runs with 
    //   no issues.
    return;
}


// goal is to write something in memory, then send a capability away for 
//    another compartment to access
void __cheri_compartment("capability_sender") entry(){
    const char *allocation = "Bananas and apples";
    Debug::log("Writing secret message at {}", &allocation);

    transfer_capability(allocation);

    Debug::log("Capability info now: {}", &allocation);
}