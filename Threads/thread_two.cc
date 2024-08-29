#include <cheri.hh>
#include <debug.hh>
#include <errno.h>
#include <thread.h>
#include <fail-simulator-on-error.h>
#include <platform-gpio.hh>


/// Expose debugging features unconditionally for this compartment.
using Debug = ConditionalDebug<true, "Thread 2">;

// Import some useful things from the CHERI namespace.
using namespace CHERI;

void __cheri_compartment("thread_two") entry()
{
    // uint16_t threadID = thread_id_get();

    Debug::log("starting thread {}",thread_id_get());


    Debug::log("Attempting to access memory at hardcoded value 0x1027f0");
    char* ptr = reinterpret_cast<char *>(0x1027f0);
    

    // trying to directly set permissions. Unsuccessful
    Capability forcedAccess = ptr;
    PermissionSet ptrPermissions = forcedAccess.permissions();
    static constexpr PermissionSet Permissions{
	  Permission::Load, Permission::Store, Permission::LoadStoreCapability};
    ptrPermissions = Permissions;
    ptr = forcedAccess;


    Debug::log("Pointer created with data: {}", ptr);



    // The pointer on this thread has no permissions, so any attempted reads
    // or writes will error
    Debug::log("Attempting to read secret message");
    Debug::log("Secret message: {}", *ptr);



   
    Debug::log("ending thread");
    return;
}