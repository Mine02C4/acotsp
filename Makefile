# Docker section
IMAGE_NAME:=acotsp

.PHONY: build rebuild start bash

build:
	docker build -t $(IMAGE_NAME) .

rebuild:
	docker build --no-cache -t $(IMAGE_NAME) .

start:
	docker run --name $(IMAGE_NAME) -it --rm $(IMAGE_NAME)

bash:
	docker run --name $(IMAGE_NAME) -it --rm --entrypoint="bash" $(IMAGE_NAME)

in_bash:
	docker exec -it $(IMAGE_NAME) bash

.PHONY: build rebuild start bash

# Inside Docker
CC := gcc
CFLAGS := -Wall -O2

OMPI_CC := mpicc
OMPI_CXX := mpic++
OMPI_CFLAGS := -Wall -O2
OMPI_LDFLAGS := -lm

SRCS := $(wildcard *.c)
DEPS := $(patsubst %.c,%.d, $(OBJS))

all: acotsp.out

run: all
	mpirun -np 4 --allow-run-as-root ./acotsp.out cities.txt

acotsp.out: acotsp.c
	$(OMPI_CC) -o "$@" -MMD -MP -MT "$@" -MF "$(patsubst %.c,%.d, $<)" $< $(OMPI_CFLAGS) $(OMPI_LDFLAGS)

citygen.out: citygen.c
	$(CC) -o "$@" -MMD -MP -MT "$@" -MF "$(patsubst %.c,%.d, $<)" $< $(CFLAGS)

-include $(DEPS)

