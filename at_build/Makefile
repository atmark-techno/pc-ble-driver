curdir = $(shell pwd)

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
	/usr/bin/c++ -g examples/monitor.c.o -o heart_rate_monitor objects/libnrf-ble-driver-sd_api_v6.a -lpthread -ludev

heart_rate_collector: objects/libnrf-ble-driver-sd_api_v6.a examples/collector.c.o
	/usr/bin/c++ -g examples/collector.c.o -o heart_rate_collector objects/libnrf-ble-driver-sd_api_v6.a -lpthread -ludev

objects/libnrf-ble-driver-sd_api_v6.a:
	$(MAKE) -f src.make build
	$(MAKE) -f libnrf-ble-driver.make build

examples/monitor.c.o:
	$(MAKE) -f heart_rate_monitor.make build

examples/collector.c.o:
	$(MAKE) -f heart_rate_collector.make build

clean:
	-rm -rf heart_rate_monitor heart_rate_collector examples/* objects/*
