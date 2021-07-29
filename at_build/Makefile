curdir = $(shell pwd)
CROSS_COMPILE =

ifeq ($(CROSS_COMPILE), arm-linux-gnueabihf-)
	CPP = /usr/bin/arm-linux-gnueabihf-g++
else
	CPP = /usr/bin/c++
endif

all: objects examples build

objects:
	@if [ ! -d objects ]; then \
		mkdir objects; \
	fi

examples:
	@if [ ! -d examples ]; then \
		mkdir examples; \
	fi

build: heart_rate_monitor heart_rate_collector

heart_rate_monitor: objects/libnrf-ble-driver-sd_api_v6.a examples/monitor.c.o
	$(CPP) -g examples/monitor.c.o -o heart_rate_monitor objects/libnrf-ble-driver-sd_api_v6.a -lpthread -ludev

heart_rate_collector: objects/libnrf-ble-driver-sd_api_v6.a examples/collector.c.o
	$(CPP) -g examples/collector.c.o -o heart_rate_collector objects/libnrf-ble-driver-sd_api_v6.a -lpthread -ludev

objects/libnrf-ble-driver-sd_api_v6.a:
	$(MAKE) CROSS_COMPILE=$(CROSS_COMPILE) -f src.make build
	$(MAKE) -f libnrf-ble-driver.make build

examples/monitor.c.o:
	$(MAKE) CROSS_COMPILE=$(CROSS_COMPILE) -f heart_rate_monitor.make build

examples/collector.c.o:
	$(MAKE) CROSS_COMPILE=$(CROSS_COMPILE) -f heart_rate_collector.make build

clean:
	-rm -rf heart_rate_monitor heart_rate_collector examples/* objects/*
