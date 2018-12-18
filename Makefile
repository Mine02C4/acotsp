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
	docker run --name $(IMAGE_NAME) -it --rm --entrypoint="bash" $(IMAGE_NAME)

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
SIMGRID_PATH := ~/approx-sim/MpiEnv/simgrid/inst
#export PATH := $(SIMGRID_PATH)/bin:$(PATH)
SMPI_CC := smpicc
SMPI_RUN := smpirun
SMPI_LDFLAGS = -L$(SIMGRID_PATH)/lib -lsimgrid -lm
SMPI_INCLUDE = -I$(SIMGRID_PATH)/include/smpi

S_XMLS := $(wildcard cases/*.xml)
S_LOGS := $(patsubst %.xml,%.log,$(S_XMLS))

simgrid: acotsp_simgrid.out

run_simgrid: $(S_LOGS)

run_simgrid2: acotsp_simgrid.out
	$(SMPI_RUN) -np 16 --cfg=smpi/privatize_global_variables:yes \
		-platform 3trandom-4-4-4-s0.seq-2.8-1-0.xml \
		-hostfile 3trandom-4-4-4-s0.seq-2.8-1-0.txt \
		./$< cities.txt | tee $<.log

acotsp_simgrid.out: acotsp.c acotsp.h
	$(SMPI_CC) -o $@ $< $(SMPI_INCLUDE) $(SMPI_LDFLAGS)

cases/%.log: cases/%.xml acotsp_simgrid.out
	$(SMPI_RUN) -np 4 --cfg=smpi/privatize_global_variables:yes \
		-platform $< \
		-hostfile cases/base.txt \
		./acotsp_simgrid.out cities.txt 2>&1 | tee $@


clean:
	-$(RM) acotsp_simgrid.out acotsp.out
	-$(RM) $(S_LOGS)

