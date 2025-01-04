.PHONY: all clean verify

# Default value if not provided
USE_BUGGY_COUNTER ?= 0

all: verify

verify:
	spin -run -DUSE_BUGGY_COUNTER=$(USE_BUGGY_COUNTER) -a update.pml

replay:
	spin -p -t -g update.pml

clean:
	rm -f pan*

