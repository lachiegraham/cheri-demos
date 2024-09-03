#include "capability_receiver.h"
#include "capability_sender.h"
#include <debug.hh>
#include <fail-simulator-on-error.h>



/// Expose debugging features unconditionally for this compartment.
using Debug = ConditionalDebug<true, "Receiver">;


// goal is to receive a memory address and access what's there
void transfer_capability(const char* allocation) {
    Debug::log("Reading message: {}", allocation);
    Debug::log("Capability info: {}", &allocation);


    Debug::log("Fiddling with capability");

    transfer_back(allocation);
    Debug::log("Capability info: {}", *(&allocation + 10));
    // The capability seems to be safe from fiddling
    //  It is put in a read_only state, and even when I force
    //   some kind of error (see above) the sender compartment runs with 
    //   no issues.
    
    return;
}

// I don't think we need a main method in this compartment