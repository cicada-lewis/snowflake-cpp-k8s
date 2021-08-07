PREFIX = ''
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
	PREFIX = '/usr/include/snowflake'
endif
ifeq ($(UNAME_S), Darwin)
	PREFIX = '/usr/local/include/snowflake'
endif

install:
	mkdir -p $(PREFIX)
	install -m 644 snowflake.h $(PREFIX)
	install -m 644 utils.h $(PREFIX)

