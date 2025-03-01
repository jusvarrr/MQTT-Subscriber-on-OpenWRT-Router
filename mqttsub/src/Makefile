LDFLAGS:=-lcurl -lmosquitto -luci -lcjson -largp -lsqlite3

SRCS := $(wildcard src/*.c)

OUTPUT := mqttsub

$(OUTPUT): $(SRCS)
	$(CC) -I./include $(CFLAGS) -o $(OUTPUT) $^ $(LDFLAGS)

clean:
	rm -f $(OUTPUT)
