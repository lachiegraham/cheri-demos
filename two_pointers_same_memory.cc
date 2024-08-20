#include <cheri.hh>
#include <debug.hh>
#include <errno.h>
#include <fail-simulator-on-error.h>
#include <platform-gpio.hh>

/// Expose debugging features unconditionally for this compartment.
using Debug = ConditionalDebug<true, "Memory safety compartment">;

// Import some useful things from the CHERI namespace.
using namespace CHERI;

void __cheri_compartment("two_pointers") entry()
{
    char *pointer1;
    int *pointer2;
    pointer1 = static_cast<char *>(malloc(256));
    pointer2 = (int *) pointer1;
    
    Debug::log("pointer1 at start {}, value {}", &pointer1, *pointer1);
    Debug::log("pointer2 at start {}, value {}", &pointer2, *pointer2);

    /* The only thing to note is that the capabilities have 
    slightly different ranges. I assume this is because
    ints are 4x the size of chars, but the char pointer is the larger.
    All permissions are the same between the two capabilities.

    pointer 1 range: 0x102710-0x102718
    pointer 2 range: 0x102708-0x102710 
    */
    for (int i = 0; i < 256/4; i++) {
        pointer2[i] = 99;
    }

    Debug::log("pointer1 now {}, value {}", &pointer1, *pointer1);
    Debug::log("pointer2 now {}, value {}", &pointer2, *pointer2);

    return;
}