// code modified from example 8 in cheriot-rtos repo
#define TEST_NAME "testing what this name is used for"
#include <cheri.hh>
#include <debug.hh>
#include <errno.h>
#include <fail-simulator-on-error.h>
#include <platform-gpio.hh>

/// Expose debugging features unconditionally for this compartment.
using Debug = ConditionalDebug<true, "Memory safety compartment">;

// Import some useful things from the CHERI namespace.
using namespace CHERI;

void __cheri_compartment("malloc_test") entry()
{
            char *allocation;
	        allocation = static_cast<char *>(malloc(256));
			free(allocation);

			/*
			 * From this point forward, any dereference of any dangling pointer
			 * to the freed memory will trap. This is guaranteed by the hardware
			 * load barrier that, on loads of capabilities to the memory region
			 * that can be used as a heap, checks the revocation bit
			 * corresponding to the base of the capability and clears the tag if
			 * it is set. For more details, see docs/architecture.md.
			 */
			allocation[0] = 'B';

			// turn LED on if we get down here
			auto gpio = MMIO_CAPABILITY(SonataGPIO, gpio);
			gpio->led_on(0);
}