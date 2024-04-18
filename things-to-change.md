## Enligt GPT

Detta svarar GPT på :We have implemented a skeleton code(rfc5905) that uses unix, however it seems like the skeleton uses functions, data structures  and libraries that FreeRTOS doesn’t support. Do you have any suggestions on howe this issues can be solved?

1. Replace Unsupported Libraries and Functions
Time Functions: FreeRTOS does not support standard UNIX time functions like gettimeofday(). You will need to use the FreeRTOS time functions such as xTaskGetTickCount() or create a custom timekeeping system using FreeRTOS timers.

Memory Management: FreeRTOS supports dynamic memory allocation through its API (e.g., pvPortMalloc(), vPortFree()), which should replace the standard C malloc() and free().

Standard Libraries: Libraries such as `<math.h>`, `<stdlib.h>`, and `<string.h>` may not be fully supported or optimized for FreeRTOS. You'll need to check each function used from these libraries for compatibility with FreeRTOS or find alternative methods provided by FreeRTOS.

2. Modify Data Structures
System Resources: Adjust data structures and system resources to be compatible with the constraints of your embedded system running FreeRTOS, focusing on minimizing RAM and CPU usage.
3. Networking and Protocol Handling
Networking Capabilities: Replace UNIX network handling functions with those supported by the LwIP stack or the native FreeRTOS+TCP, depending on which one your setup supports.

Packet Handling: Modify the packet handling sections of your code to align with how FreeRTOS handles packet send/receive operations.

4. Concurrency and Task Management
Multithreading: Convert processes and threads into FreeRTOS tasks. You'll need to manage task priorities and stack sizes carefully to ensure system stability.

Synchronization: Use FreeRTOS synchronization primitives like semaphores, mutexes, or event groups to manage access to shared resources.

5. Configuration and Optimization
Configuration: Configure FreeRTOS settings to optimize performance and resource usage, which may involve tuning the FreeRTOSConfig.h file to better suit your application's needs.

Optimization: Consider the computational requirements and optimize the algorithm to reduce CPU usage, possibly by simplifying calculations or reducing precision where acceptable.

6. Debugging and Testing
Debugging: Extensively test the modified system in a real-time operating environment to identify and fix issues related to timing, task management, and resource allocation.

Validation: Ensure that the implementation still adheres to the standards defined by RFC5905 by validating the protocol's behavior in various scenarios.


## Vad jag sett i sklettkoden så måste vi ändra:

* gettimeofday() ska kunna ersättas med xTaskGetTickCount() eller genom att använda FreeRTOS timers.

* malloc() ska kunna ersättas med pvPortMalloc().

* free() ska kunna ersättas med vPortFree().

* `<math.h>`, `<stdlib.h>`, och `<string.h>` kan eventuellt behöva bytas, FreeRTOS har eventuelt allternativa funktioner för de som använder dessa. 
