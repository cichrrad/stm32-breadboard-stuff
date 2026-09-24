# To compile this, there are added dependencies due to open62541
# run:

# sudo apt-get update ; sudo apt-get install libmbedtls-dev

# in the devcontainer

BINARY_NAME=sensor_gateway

INC_DIRS := . \
            $(shell find ./local_source -type d)

SRCS := $(wildcard *.c) \
        $(shell find ./local_source -name "*.c")

INC_FLAGS := $(addprefix -I,$(INC_DIRS))

# Open62541 requires threading + mbedTLS
LIBS := -pthread -lmbedtls -lmbedcrypto -lmbedx509

build:
	gcc $(INC_FLAGS) $(SRCS) -o $(BINARY_NAME) $(LIBS)

run:
	./$(BINARY_NAME)