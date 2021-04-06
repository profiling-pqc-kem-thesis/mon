source := $(shell find src -type f -iname '*.c')
headers := $(shell find src -type f -iname '*.h')

CFLAGS += -no-pie

all: build/mon build/test

build/mon: $(source) $(headers)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $(source)

build/test: test/target.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -rf build &>/dev/null || true
