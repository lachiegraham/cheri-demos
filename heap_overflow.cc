// code modified from example 8 in cheriot-rtos repo
#define TEST_NAME "Overflow test"
#include <cheri.hh>
#include <debug.hh>
#include <errno.h>
#include <fail-simulator-on-error.h>
#include <platform-gpio.hh>

/// Expose debugging features unconditionally for this compartment.
using Debug = ConditionalDebug<true, "Memory safety compartment">;

// Import some useful things from the CHERI namespace.
using namespace CHERI;

void __cheri_compartment("overflow_entry") entry()
{
	Debug::log("Starting stack overflow test");
	/*
			 * Trigger a linear overflow on the heap, by storing one byte beyond
			 * an allocation bounds. The bounds checks are performed in the
			 * architectural level, by the CPU. Each capability carries the
			 * allocation bounds.
			 */
            int length = 256;
            char *allocation;
			allocation = static_cast<char *>(malloc(length));
			Debug::Assert(allocation != NULL, "Allocation failed");

			Debug::log("Trigger heap linear overflow");
			allocation[length] = 'J';


			// turn LED on if we get down here
			auto gpio = MMIO_CAPABILITY(SonataGPIO, gpio);
			gpio->led_on(0);

			Debug::Assert(false, "Code after overflow should be unreachable");
}
