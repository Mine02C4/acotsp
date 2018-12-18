# Docker section
IMAGE_NAME:=acotsp

.PHONY: build rebuild start bash clean

build:
	docker build -t $(IMAGE_NAME) .

rebuild:
	docker build --no-cache -t $(IMAGE_NAME) .

start:
	docker run --name $(IMAGE_NAME) -it --rm $(IMAGE_NAME)

bash:
	docker exec -it $(IMAGE_NAME) bash

# Inside Docker
OMPI_CC := mpicc
OMPI_CXX := mpic++
OMPI_CFLAGS := -Wall -O2
OMPI_LDFLAGS := -lm

all: acotsp.out

run: all
	mpirun -np 4 --allow-run-as-root ./acotsp.out cities.txt

acotsp.out: acotsp.c acotsp.h
	$(OMPI_CC) -o $@ $< $(OMPI_CFLAGS) $(OMPI_LDFLAGS)

# For simgrid
MPIENVPREFIX=~/approx-sim/MpiEnv
SIMGRID_PATH := ${MPIENVPREFIX}/simgrid/inst
SMPI_CC := ${SIMGRID_PATH}/bin/smpicc
SMPI_RUN := ${SIMGRID_PATH}/bin/smpirun
SMPI_LDFLAGS = -L$(SIMGRID_PATH)/lib -lsimgrid -lm
SMPI_INCLUDE = -I${SIMGRID_PATH}/include/smpi

simgrid: acotsp_simgrid.out

run_simgrid: acotsp_simgrid.out
	$(SMPI_RUN) -np 4 --cfg=smpi/privatize_global_variables:yes \
		-platform 3trandom-4-4-4-s0.seq-2.8-1-0.xml \
		-hostfile 3trandom-4-4-4-s0.seq-2.8-1-0.txt \
		./$< cities.txt

acotsp_simgrid.out: acotsp.c acotsp.h
	$(SMPI_CC) -o $@ $< $(SMPI_INCLUDE) $(SMPI_LDFLAGS)

clean:
	-$(RM) acotsp_simgrid.out acotsp.out

