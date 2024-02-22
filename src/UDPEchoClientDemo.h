
/*
 * Create the UDP echo client tasks.  This is the version where an echo request
 * is made from the same task that listens for the echo reply.
 */
void vStartUDPEchoClientTasks_SingleTasks(uint16_t usTaskStackSize,
                                          UBaseType_t uxTaskPriority);
