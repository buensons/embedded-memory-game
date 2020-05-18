CC=$(CROSS_COMPILE)gcc
OBJS := memory_game.o

memory: $(OBJS)
	$(CC) -o memory $(CFLAGS) $(LDFLAGS) $(OBJS) -lgpiod
$(OBJS) : %.o : %.c
	$(CC) -c $(CFLAGS) -lgpiod $< -o $@
