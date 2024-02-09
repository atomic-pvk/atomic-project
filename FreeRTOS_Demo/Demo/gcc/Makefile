# Compiler and Tool Definitions
CC = arm-none-eabi-gcc
LD = arm-none-eabi-gcc
SIZE = arm-none-eabi-size
MAKE = make

# Output Directory
OUTPUT_DIR := ./output

# FreeRTOS Directories
FREERTOS_ROOT = ./../../
KERNEL_DIR = $(FREERTOS_ROOT)/Source/kernel
TCP_DIR = $(FREERTOS_ROOT)/Source/tcp 
DEMO_ROOT = $(FREERTOS_ROOT)
DEMO_PROJECT = $(DEMO_ROOT)/Demo

# Include Paths
INCLUDE_DIRS += -I$(KERNEL_DIR)/include \
                -I$(TCP_DIR)/include \
                -I$(DEMO_PROJECT) -I$(DEMO_PROJECT)/CMSIS \
                -ICMSIS

# Source Files
PATH += $(KERNEL_DIR) $(KERNEL_PORT_DIR) $(KERNEL_DIR)/portable/MemMang
SOURCE_FILES += $(KERNEL_DIR)/tasks.c
SOURCE_FILES += $(KERNEL_DIR)/list.c
SOURCE_FILES += $(KERNEL_DIR)/queue.c
SOURCE_FILES += $(KERNEL_DIR)/timers.c
SOURCE_FILES += $(KERNEL_DIR)/event_groups.c
SOURCE_FILES += $(KERNEL_DIR)/stream_buffer.c
SOURCE_FILES += $(KERNEL_DIR)/portable/MemMang/heap_4.c
SOURCE_FILES += $(KERNEL_DIR)/portable/GCC/ARM_CM3/port.c

DEMO_ROOT = $(FREERTOS_ROOT)
DEMO_KERNEL = $(DEMO_ROOT)/Source/kernel
INCLUDE_DIRS += -I$(DEMO_KERNEL)
VPATH += $(DEMO_KERNEL)
SOURCE_FILES += (DEMO_KERNEL)/AbortDelay.c
SOURCE_FILES += (DEMO_KERNEL)/BlockQ.c
SOURCE_FILES += (DEMO_KERNEL)/blocktim.c
SOURCE_FILES += (DEMO_KERNEL)/countsem.c
SOURCE_FILES += (DEMO_KERNEL)/death.c
SOURCE_FILES += (DEMO_KERNEL)/dynamic.c
SOURCE_FILES += (DEMO_KERNEL)/EventGroupsDemo.c
SOURCE_FILES += (DEMO_KERNEL)/GenQTest.c
SOURCE_FILES += (DEMO_KERNEL)/integer.c
SOURCE_FILES += (DEMO_KERNEL)/IntQueue.c
SOURCE_FILES += (DEMO_KERNEL)/IntQueueTimer.c
SOURCE_FILES += (DEMO_KERNEL)/IntSemTest.c
SOURCE_FILES += (DEMO_KERNEL)/MessageBufferAMP.c
SOURCE_FILES += (DEMO_KERNEL)/MessageBufferDemo.c
SOURCE_FILES += (DEMO_KERNEL)/PollQ.c
SOURCE_FILES += (DEMO_KERNEL)/QPeek.c
SOURCE_FILES += (DEMO_KERNEL)/QueueOverwrite.c
SOURCE_FILES += (DEMO_KERNEL)/QueueSet.c
SOURCE_FILES += (DEMO_KERNEL)/QueueSetPolling.c
SOURCE_FILES += (DEMO_KERNEL)/recmutex.c
SOURCE_FILES += (DEMO_KERNEL)/semtest.c
SOURCE_FILES += (DEMO_KERNEL)/StaticAllocation.c
SOURCE_FILES += (DEMO_KERNEL)/StreamBufferDemo.c
SOURCE_FILES += (DEMO_KERNEL)/StreamBufferInterrupt.c
SOURCE_FILES += (DEMO_KERNEL)/TaskNotify.c
SOURCE_FILES += (DEMO_KERNEL)/TaskNotifyArray.c
SOURCE_FILES += (DEMO_KERNEL)/TimerDemo.c

#
# Application entry point.  main_blinky is self contained.  main_full builds
# the above common demo (and test) files too.
#
DEMO_PROJECT = $(DEMO_ROOT)/Demo
VPATH += $(DEMO_PROJECT)
INCLUDE_DIRS += -I$(DEMO_PROJECT) -I$(DEMO_PROJECT)/CMSIS
SOURCE_FILES += $(DEMO_PROJECT)/main.c
SOURCE_FILES += $(DEMO_PROJECT)/main_blinky.c
SOURCE_FILES += $(DEMO_PROJECT)/main_full.c
SOURCE_FILES += ./startup_gcc.c
# Lightweight print formatting to use in place of the heavier GCC equivalent.
SOURCE_FILES += ./printf-stdarg.c

INCLUDE_DIRS += -I.

# Demo Source Files
SOURCE_FILES += $(DEMO_PROJECT)/startup.c
SOURCE_FILES += $(DEMO_PROJECT)/syscalls.c
SOURCE_FILES += $(DEMO_PROJECT)/main.c
SOURCE_FILES += $(DEMO_PROJECT)/main_networking.c
SOURCE_FILES += $(DEMO_PROJECT)/TCPEchoClient_SingleTasks.c

INCLUDE_DIRS += -ICMSIS

# FreeRTOS Kernel ARM Cortex-M3 Port
INCLUDE_DIRS += -I$(KERNEL_DIR)/portable/GCC/ARM_CM3

# FreeRTOS+TCP
INCLUDE_DIRS += -I${TCP_DIR}/include/

SOURCE_FILES += ${TCP_DIR}/FreeRTOS_ARP.c
SOURCE_FILES += ${TCP_DIR}/FreeRTOS_DHCP.c
SOURCE_FILES += ${TCP_DIR}/FreeRTOS_DNS.c
SOURCE_FILES += ${TCP_DIR}/FreeRTOS_DNS_Cache.c
SOURCE_FILES += ${TCP_DIR}/FreeRTOS_DNS_Callback.c
SOURCE_FILES += ${TCP_DIR}/FreeRTOS_DNS_Networking.c
SOURCE_FILES += ${TCP_DIR}/FreeRTOS_DNS_Parser.c
SOURCE_FILES += ${TCP_DIR}/FreeRTOS_ICMP.c
SOURCE_FILES += ${TCP_DIR}/FreeRTOS_IP.c
SOURCE_FILES += ${TCP_DIR}/FreeRTOS_IP_Timers.c
SOURCE_FILES += ${TCP_DIR}/FreeRTOS_IP_Utils.c
SOURCE_FILES += ${TCP_DIR}/FreeRTOS_Sockets.c
SOURCE_FILES += ${TCP_DIR}/FreeRTOS_Stream_Buffer.c
SOURCE_FILES += ${TCP_DIR}/FreeRTOS_TCP_IP.c
SOURCE_FILES += ${TCP_DIR}/FreeRTOS_TCP_Reception.c
SOURCE_FILES += ${TCP_DIR}/FreeRTOS_TCP_State_Handling.c
SOURCE_FILES += ${TCP_DIR}/FreeRTOS_TCP_Transmission.c
SOURCE_FILES += ${TCP_DIR}/FreeRTOS_TCP_Utils.c
SOURCE_FILES += ${TCP_DIR}/FreeRTOS_TCP_WIN.c
SOURCE_FILES += ${TCP_DIR}/FreeRTOS_Tiny_TCP.c
SOURCE_FILES += ${TCP_DIR}/FreeRTOS_UDP_IP.c

# FreeRTOS+TCP Port for ARM MPS2 SoC
INCLUDE_DIRS += -I${TCP_DIR}/portable/NetworkInterface/MPS2_AN385/ether_lan9118
INCLUDE_DIRS += -I${TCP_DIR}/portable/Compiler/GCC

SOURCE_FILES += ${TCP_DIR}/portable/BufferManagement/BufferAllocation_2.c
SOURCE_FILES += ${TCP_DIR}/portable/NetworkInterface/MPS2_AN385/NetworkInterface.c
SOURCE_FILES += ${TCP_DIR}/portable/NetworkInterface/MPS2_AN385/ether_lan9118/smsc9220_eth_drv.c

# Compile Flags
CFLAGS += -nostartfiles -mthumb -mcpu=cortex-m3 -Wno-error=implicit-function-declaration \
          -Wno-builtin-declaration-mismatch -Werror -ffreestanding \
          -Wall -Wextra -g3 -O0 -ffunction-sections -fdata-sections \
          -MMD -MP -MF"$(@:%.o=%.d)" -MT $@

# Object File and Dependency Rules
OBJS = $(SOURCE_FILES:%.c=%.o)
OBJS_NO_PATH = $(notdir $(OBJS))
OBJS_OUTPUT = $(OBJS_NO_PATH:%.o=$(OUTPUT_DIR)/%.o)
DEP_FILES := $(SOURCE_FILES:%.c=$(OUTPUT_DIR)/%.d)
DEP_FILES_NO_PATH = $(notdir $(DEP_FILES))
DEP_OUTPUT = $(DEP_FILES_NO_PATH:%.d=$(OUTPUT_DIR)/%.d)

# Linker Flags and Rules
LDFLAGS = -T mps2_m3.ld -specs=nano.specs --specs=rdimon.specs -lc -lrdimon \
          -Xlinker --gc-sections -Xlinker -T ./mps2_m3.ld \
          -Xlinker -Map=$(OUTPUT_DIR)/RTOSDemo.map

# Targets
all: $(OUTPUT_DIR)/$(IMAGE)

$(OUTPUT_DIR)/%.o : %.c $(OUTPUT_DIR)/%.d Makefile
    $(CC) $(CFLAGS) -c $< -o $@

$(OUTPUT_DIR)/$(IMAGE): ./mps2_m3.ld $(OBJS_OUTPUT) Makefile
    $(LD) $(OBJS_OUTPUT) $(CFLAGS) $(LDFLAGS) -o $(OUTPUT_DIR)/$(IMAGE)
    $(SIZE) $(OUTPUT_DIR)/$(IMAGE) 

$(DEP_OUTPUT):
include $(wildcard $(DEP_OUTPUT))

clean:
    rm -f $(OUTPUT_DIR)/$(IMAGE) $(OUTPUT_DIR)/*.o $(OUTPUT_DIR)/*.d

# Misc
print-%  : ; @echo $* = $($*)
.PHONY: all clean
