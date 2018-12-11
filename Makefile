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
	docker exec -it $(IMAGE_NAME) bash

.PHONY: build rebuild start bash

# Inside Docker
OMPI_CC := mpicc
OMPI_CXX := mpic++
CFLAGS := -Wall -O2
LDFLAGS := -lm

all: acotsp.out

run: all
	mpirun -np 4 --allow-run-as-root ./acotsp.out cities.txt

acotsp.out: acotsp.c acotsp.h
	$(OMPI_CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

