

CC = $(CROSS_COMPILE)gcc
CFLAGS += -Wall -I ../../.. -I lib/include -D RECONOS_OS_linux -D RECONOS_BOARD_zedboard_d -I $(RECONOS)/linux/tools/timer -I application
LDFLAGS += -static -lpthread -L $(RECONOS)/linux/tools/timer -ltimer -lm

MKDIRP = mkdir -p

OBJS = rt_inverse/rt_inverse.o rt_control/rt_control.o rt_saw/rt_saw.o rt_power/rt_power.o application/main.o rt_web/rt_web.o lib/runtime/reconos.o lib/runtime/reconos_app.o lib/runtime/comp/rqueue.o lib/runtime/comp/mbox.o lib/runtime/arch/arch_linux.o lib/runtime/arch/arch_xilkernel.o 
BUILD_OBJS = $(OBJS:%=_build/%)

all: $(BUILD_OBJS)
	$(CC) $(BUILD_OBJS) $(LDFLAGS) -o recobop

install: all
	cp recobop $(PREFIX)

clean:
	rm -rf _build recobop

_build/%.o: %.c
	$(MKDIRP) $(@D)
	$(CC) $(CFLAGS) -x c -c $< -o $@

_build/%.o: %.cpp
	$(MKDIRP) $(@D)
	$(CC) $(CFLAGS) -x c -c $< -o $@
