curdir = $(shell pwd)
CROSS_COMPILE =
TARGETS = heart_rate_monitor \
		  heart_rate_collector \
		  test_peripheral \
		  test_central \
		  test_throughput \
		  scan_sample

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

build: $(TARGETS)

heart_rate_monitor: objects examples objects/libnrf-ble-driver-sd_api_v6.a examples/monitor.c.o
	$(CPP) -g examples/monitor.c.o -o heart_rate_monitor objects/libnrf-ble-driver-sd_api_v6.a -lpthread -ludev

heart_rate_collector: objects examples objects/libnrf-ble-driver-sd_api_v6.a examples/collector.c.o
	$(CPP) -g examples/collector.c.o -o heart_rate_collector objects/libnrf-ble-driver-sd_api_v6.a -lpthread -ludev

test_peripheral: objects examples objects/libnrf-ble-driver-sd_api_v6.a examples/test_peripheral.c.o
	$(CPP) -g examples/test_peripheral.c.o -o test_peripheral objects/libnrf-ble-driver-sd_api_v6.a -lpthread -ludev

test_central: objects examples objects/libnrf-ble-driver-sd_api_v6.a examples/test_central.c.o
	$(CPP) -g examples/test_central.c.o -o test_central objects/libnrf-ble-driver-sd_api_v6.a -lpthread -ludev

test_throughput: objects examples objects/libnrf-ble-driver-sd_api_v6.a examples/test_throughput.c.o
	$(CPP) -g examples/test_throughput.c.o examples/amts.c.o examples/amtc.c.o -o test_throughput objects/libnrf-ble-driver-sd_api_v6.a -lpthread -ludev

scan_sample: objects examples objects/libnrf-ble-driver-sd_api_v6.a examples/scan_sample.c.o
	$(CPP) -g examples/scan_sample.c.o -o scan_sample objects/libnrf-ble-driver-sd_api_v6.a -lpthread -ludev

objects/libnrf-ble-driver-sd_api_v6.a:
	$(MAKE) CROSS_COMPILE=$(CROSS_COMPILE) -f src.make build
	$(MAKE) -f libnrf-ble-driver.make build

examples/monitor.c.o:
	$(MAKE) CROSS_COMPILE=$(CROSS_COMPILE) -f heart_rate_monitor.make build

examples/collector.c.o:
	$(MAKE) CROSS_COMPILE=$(CROSS_COMPILE) -f heart_rate_collector.make build

examples/test_peripheral.c.o:
	$(MAKE) CROSS_COMPILE=$(CROSS_COMPILE) -f test_peripheral.make build

examples/test_central.c.o:
	$(MAKE) CROSS_COMPILE=$(CROSS_COMPILE) -f test_central.make build

examples/test_throughput.c.o: ../examples/test_throughput/main.c
	$(MAKE) CROSS_COMPILE=$(CROSS_COMPILE) -f test_throughput.make build

examples/scan_sample.c.o:
	$(MAKE) CROSS_COMPILE=$(CROSS_COMPILE) -f scan_sample.make build

clean:
	-rm -rf $(TARGETS) examples/*

dist-clean:
	-rm -rf $(TARGETS) examples objects
