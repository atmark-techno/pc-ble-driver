#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

curdir := $(shell pwd)

# compile C with /usr/bin/cc
# compile CXX with /usr/bin/c++
C_FLAGS = -g -fPIC   -Wall -Wno-unknown-pragmas -Wno-undef -Wno-long-long -Wfloat-equal -Wpointer-arith -Wlogical-op -std=c99

C_DEFINES = -DASIO_STANDALONE -DHCI_LINK_CONTROL -DNRF_SD_BLE_API_VERSION=6 -DSD_RPC_EXPORTS

C_INCLUDES = -isystem $(curdir)/../src/sd_api_common/sdk/components/libraries/util -isystem $(curdir)/../src/sd_api_v6/sdk/components/serialization/application/codecs/common -isystem $(curdir)/../src/sd_api_v6/sdk/components/serialization/application/codecs/s132/serializers -isystem $(curdir)/../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers -isystem $(curdir)/../src/sd_api_common/sdk/components/serialization/common -isystem $(curdir)/../src/sd_api_common/sdk/components/serialization/common/struct_ser/ble -isystem $(curdir)/../include/common -isystem $(curdir)/../include/common/internal -isystem $(curdir)/../include/common/internal/transport -isystem /home/akihito/git/vcpkg/installed/x64-linux/include -isystem $(curdir)/../include/common/config -isystem $(curdir)/../include/common/sdk_compat -isystem $(curdir)/../include/sd_api_v6

CXX_FLAGS = -g -fPIC   -Wall -Wno-unknown-pragmas -Wno-undef -Wno-long-long -Wfloat-equal -Wpointer-arith -Wlogical-op -std=c++14 -std=gnu++14

CXX_DEFINES = -DASIO_STANDALONE -DHCI_LINK_CONTROL -DNRF_SD_BLE_API_VERSION=6 -DSD_RPC_EXPORTS

CXX_INCLUDES = -isystem $(curdir)/../src/sd_api_common/sdk/components/libraries/util -isystem $(curdir)/../src/sd_api_v6/sdk/components/serialization/application/codecs/common -isystem $(curdir)/../src/sd_api_v6/sdk/components/serialization/application/codecs/s132/serializers -isystem $(curdir)/../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers -isystem $(curdir)/../src/sd_api_common/sdk/components/serialization/common -isystem $(curdir)/../src/sd_api_common/sdk/components/serialization/common/struct_ser/ble -isystem $(curdir)/../include/common -isystem $(curdir)/../include/common/internal -isystem $(curdir)/../include/common/internal/transport -isystem /home/akihito/git/vcpkg/installed/x64-linux/include -isystem $(curdir)/../include/common/config -isystem $(curdir)/../include/common/sdk_compat -isystem $(curdir)/../include/sd_api_v6

../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/app_ble_user_mem.c.o: ../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/app_ble_user_mem.c
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o objects/app_ble_user_mem.c.o   -c $(curdir)/../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/app_ble_user_mem.c

../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/app_ble_user_mem.c.i:
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E $(curdir)/../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/app_ble_user_mem.c > objects/app_ble_user_mem.c.i

../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/app_ble_user_mem.c.s:
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S $(curdir)/../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/app_ble_user_mem.c -o objects/app_ble_user_mem.c.s

../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_app.c.o: ../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_app.c
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o objects/ble_app.c.o   -c $(curdir)/../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_app.c

../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_app.c.i:
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E $(curdir)/../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_app.c > objects/ble_app.c.i

../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_app.c.s:
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S $(curdir)/../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_app.c -o objects/ble_app.c.s

../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_event.c.o: ../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_event.c
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o objects/ble_event.c.o   -c $(curdir)/../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_event.c

../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_event.c.i:
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E $(curdir)/../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_event.c > objects/ble_event.c.i

../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_event.c.s:
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S $(curdir)/../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_event.c -o objects/ble_event.c.s

../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_evt_app.c.o: ../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_evt_app.c
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o objects/ble_evt_app.c.o   -c $(curdir)/../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_evt_app.c

../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_evt_app.c.i:
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E $(curdir)/../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_evt_app.c > objects/ble_evt_app.c.i

../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_evt_app.c.s:
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S $(curdir)/../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_evt_app.c -o objects/ble_evt_app.c.s

../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_gap_app.c.o: ../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_gap_app.c
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o objects/ble_gap_app.c.o   -c $(curdir)/../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_gap_app.c

../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_gap_app.c.i:
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E $(curdir)/../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_gap_app.c > objects/ble_gap_app.c.i

../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_gap_app.c.s:
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S $(curdir)/../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_gap_app.c -o objects/ble_gap_app.c.s

../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_gap_evt_app.c.o: ../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_gap_evt_app.c
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o objects/ble_gap_evt_app.c.o   -c $(curdir)/../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_gap_evt_app.c

../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_gap_evt_app.c.i:
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E $(curdir)/../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_gap_evt_app.c > objects/ble_gap_evt_app.c.i

../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_gap_evt_app.c.s:
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S $(curdir)/../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_gap_evt_app.c -o objects/ble_gap_evt_app.c.s

../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_gattc_app.c.o: ../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_gattc_app.c
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o objects/ble_gattc_app.c.o   -c $(curdir)/../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_gattc_app.c

../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_gattc_app.c.i:
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E $(curdir)/../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_gattc_app.c > objects/ble_gattc_app.c.i

../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_gattc_app.c.s:
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S $(curdir)/../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_gattc_app.c -o objects/ble_gattc_app.c.s

../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_gattc_evt_app.c.o: ../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_gattc_evt_app.c
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o objects/ble_gattc_evt_app.c.o   -c $(curdir)/../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_gattc_evt_app.c

../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_gattc_evt_app.c.i:
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E $(curdir)/../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_gattc_evt_app.c > objects/ble_gattc_evt_app.c.i

../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_gattc_evt_app.c.s:
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S $(curdir)/../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_gattc_evt_app.c -o objects/ble_gattc_evt_app.c.s

../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_gatts_app.c.o: ../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_gatts_app.c
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o objects/ble_gatts_app.c.o   -c $(curdir)/../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_gatts_app.c

../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_gatts_app.c.i:
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E $(curdir)/../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_gatts_app.c > objects/ble_gatts_app.c.i

../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_gatts_app.c.s:
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S $(curdir)/../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_gatts_app.c -o objects/ble_gatts_app.c.s

../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_gatts_evt_app.c.o: ../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_gatts_evt_app.c
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o objects/ble_gatts_evt_app.c.o   -c $(curdir)/../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_gatts_evt_app.c

../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_gatts_evt_app.c.i:
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E $(curdir)/../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_gatts_evt_app.c > objects/ble_gatts_evt_app.c.i

../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_gatts_evt_app.c.s:
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S $(curdir)/../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_gatts_evt_app.c -o objects/ble_gatts_evt_app.c.s

../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_l2cap_app.c.o: ../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_l2cap_app.c
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o objects/ble_l2cap_app.c.o   -c $(curdir)/../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_l2cap_app.c

../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_l2cap_app.c.i:
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E $(curdir)/../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_l2cap_app.c > objects/ble_l2cap_app.c.i

../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_l2cap_app.c.s:
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S $(curdir)/../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_l2cap_app.c -o objects/ble_l2cap_app.c.s

../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_l2cap_evt_app.c.o: ../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_l2cap_evt_app.c
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o objects/ble_l2cap_evt_app.c.o   -c $(curdir)/../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_l2cap_evt_app.c

../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_l2cap_evt_app.c.i:
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E $(curdir)/../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_l2cap_evt_app.c > objects/ble_l2cap_evt_app.c.i

../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_l2cap_evt_app.c.s:
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S $(curdir)/../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_l2cap_evt_app.c -o objects/ble_l2cap_evt_app.c.s

../src/sd_api_v6/sdk/components/serialization/application/codecs/common/conn_systemreset_app.c.o: ../src/sd_api_v6/sdk/components/serialization/application/codecs/common/conn_systemreset_app.c
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o objects/conn_systemreset_app.c.o   -c $(curdir)/../src/sd_api_v6/sdk/components/serialization/application/codecs/common/conn_systemreset_app.c

../src/sd_api_v6/sdk/components/serialization/application/codecs/common/conn_systemreset_app.c.i:
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E $(curdir)/../src/sd_api_v6/sdk/components/serialization/application/codecs/common/conn_systemreset_app.c > objects/conn_systemreset_app.c.i

../src/sd_api_v6/sdk/components/serialization/application/codecs/common/conn_systemreset_app.c.s:
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S $(curdir)/../src/sd_api_v6/sdk/components/serialization/application/codecs/common/conn_systemreset_app.c -o objects/conn_systemreset_app.c.s

../src/sd_api_common/sdk/components/serialization/common/ble_serialization.c.o: ../src/sd_api_common/sdk/components/serialization/common/ble_serialization.c
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o objects/ble_serialization.c.o   -c $(curdir)/../src/sd_api_common/sdk/components/serialization/common/ble_serialization.c

../src/sd_api_common/sdk/components/serialization/common/ble_serialization.c.i:
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E $(curdir)/../src/sd_api_common/sdk/components/serialization/common/ble_serialization.c > objects/ble_serialization.c.i

../src/sd_api_common/sdk/components/serialization/common/ble_serialization.c.s:
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S $(curdir)/../src/sd_api_common/sdk/components/serialization/common/ble_serialization.c -o objects/ble_serialization.c.s

../src/sd_api_common/sdk/components/serialization/common/cond_field_serialization.c.o: ../src/sd_api_common/sdk/components/serialization/common/cond_field_serialization.c
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o objects/cond_field_serialization.c.o   -c $(curdir)/../src/sd_api_common/sdk/components/serialization/common/cond_field_serialization.c

../src/sd_api_common/sdk/components/serialization/common/cond_field_serialization.c.i:
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E $(curdir)/../src/sd_api_common/sdk/components/serialization/common/cond_field_serialization.c > objects/cond_field_serialization.c.i

../src/sd_api_common/sdk/components/serialization/common/cond_field_serialization.c.s:
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S $(curdir)/../src/sd_api_common/sdk/components/serialization/common/cond_field_serialization.c -o objects/cond_field_serialization.c.s

../src/sd_api_common/sdk/components/serialization/common/struct_ser/ble/ble_gap_struct_serialization.c.o: ../src/sd_api_common/sdk/components/serialization/common/struct_ser/ble/ble_gap_struct_serialization.c
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o objects/ble_gap_struct_serialization.c.o   -c $(curdir)/../src/sd_api_common/sdk/components/serialization/common/struct_ser/ble/ble_gap_struct_serialization.c

../src/sd_api_common/sdk/components/serialization/common/struct_ser/ble/ble_gap_struct_serialization.c.i:
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E $(curdir)/../src/sd_api_common/sdk/components/serialization/common/struct_ser/ble/ble_gap_struct_serialization.c > objects/ble_gap_struct_serialization.c.i

../src/sd_api_common/sdk/components/serialization/common/struct_ser/ble/ble_gap_struct_serialization.c.s:
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S $(curdir)/../src/sd_api_common/sdk/components/serialization/common/struct_ser/ble/ble_gap_struct_serialization.c -o objects/ble_gap_struct_serialization.c.s

../src/sd_api_common/sdk/components/serialization/common/struct_ser/ble/ble_gattc_struct_serialization.c.o: ../src/sd_api_common/sdk/components/serialization/common/struct_ser/ble/ble_gattc_struct_serialization.c
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o objects/ble_gattc_struct_serialization.c.o   -c $(curdir)/../src/sd_api_common/sdk/components/serialization/common/struct_ser/ble/ble_gattc_struct_serialization.c

../src/sd_api_common/sdk/components/serialization/common/struct_ser/ble/ble_gattc_struct_serialization.c.i:
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E $(curdir)/../src/sd_api_common/sdk/components/serialization/common/struct_ser/ble/ble_gattc_struct_serialization.c > objects/ble_gattc_struct_serialization.c.i

../src/sd_api_common/sdk/components/serialization/common/struct_ser/ble/ble_gattc_struct_serialization.c.s:
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S $(curdir)/../src/sd_api_common/sdk/components/serialization/common/struct_ser/ble/ble_gattc_struct_serialization.c -o objects/ble_gattc_struct_serialization.c.s

../src/sd_api_common/sdk/components/serialization/common/struct_ser/ble/ble_gatts_struct_serialization.c.o: ../src/sd_api_common/sdk/components/serialization/common/struct_ser/ble/ble_gatts_struct_serialization.c
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o objects/ble_gatts_struct_serialization.c.o   -c $(curdir)/../src/sd_api_common/sdk/components/serialization/common/struct_ser/ble/ble_gatts_struct_serialization.c

../src/sd_api_common/sdk/components/serialization/common/struct_ser/ble/ble_gatts_struct_serialization.c.i:
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E $(curdir)/../src/sd_api_common/sdk/components/serialization/common/struct_ser/ble/ble_gatts_struct_serialization.c > objects/ble_gatts_struct_serialization.c.i

../src/sd_api_common/sdk/components/serialization/common/struct_ser/ble/ble_gatts_struct_serialization.c.s:
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S $(curdir)/../src/sd_api_common/sdk/components/serialization/common/struct_ser/ble/ble_gatts_struct_serialization.c -o objects/ble_gatts_struct_serialization.c.s

../src/sd_api_common/sdk/components/serialization/common/struct_ser/ble/ble_gatt_struct_serialization.c.o: ../src/sd_api_common/sdk/components/serialization/common/struct_ser/ble/ble_gatt_struct_serialization.c
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o objects/ble_gatt_struct_serialization.c.o   -c $(curdir)/../src/sd_api_common/sdk/components/serialization/common/struct_ser/ble/ble_gatt_struct_serialization.c

../src/sd_api_common/sdk/components/serialization/common/struct_ser/ble/ble_gatt_struct_serialization.c.i:
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E $(curdir)/../src/sd_api_common/sdk/components/serialization/common/struct_ser/ble/ble_gatt_struct_serialization.c > objects/ble_gatt_struct_serialization.c.i

../src/sd_api_common/sdk/components/serialization/common/struct_ser/ble/ble_gatt_struct_serialization.c.s:
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S $(curdir)/../src/sd_api_common/sdk/components/serialization/common/struct_ser/ble/ble_gatt_struct_serialization.c -o objects/ble_gatt_struct_serialization.c.s

../src/sd_api_common/sdk/components/serialization/common/struct_ser/ble/ble_l2cap_struct_serialization.c.o: ../src/sd_api_common/sdk/components/serialization/common/struct_ser/ble/ble_l2cap_struct_serialization.c
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o objects/ble_l2cap_struct_serialization.c.o   -c $(curdir)/../src/sd_api_common/sdk/components/serialization/common/struct_ser/ble/ble_l2cap_struct_serialization.c

../src/sd_api_common/sdk/components/serialization/common/struct_ser/ble/ble_l2cap_struct_serialization.c.i:
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E $(curdir)/../src/sd_api_common/sdk/components/serialization/common/struct_ser/ble/ble_l2cap_struct_serialization.c > objects/ble_l2cap_struct_serialization.c.i

../src/sd_api_common/sdk/components/serialization/common/struct_ser/ble/ble_l2cap_struct_serialization.c.s:
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S $(curdir)/../src/sd_api_common/sdk/components/serialization/common/struct_ser/ble/ble_l2cap_struct_serialization.c -o objects/ble_l2cap_struct_serialization.c.s

../src/sd_api_common/sdk/components/serialization/common/struct_ser/ble/ble_struct_serialization.c.o: ../src/sd_api_common/sdk/components/serialization/common/struct_ser/ble/ble_struct_serialization.c
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o objects/ble_struct_serialization.c.o   -c $(curdir)/../src/sd_api_common/sdk/components/serialization/common/struct_ser/ble/ble_struct_serialization.c

../src/sd_api_common/sdk/components/serialization/common/struct_ser/ble/ble_struct_serialization.c.i:
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E $(curdir)/../src/sd_api_common/sdk/components/serialization/common/struct_ser/ble/ble_struct_serialization.c > objects/ble_struct_serialization.c.i

../src/sd_api_common/sdk/components/serialization/common/struct_ser/ble/ble_struct_serialization.c.s:
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S $(curdir)/../src/sd_api_common/sdk/components/serialization/common/struct_ser/ble/ble_struct_serialization.c -o objects/ble_struct_serialization.c.s

../src/common/adapter_internal.cpp.o: ../src/common/adapter_internal.cpp
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o objects/adapter_internal.cpp.o -c $(curdir)/../src/common/adapter_internal.cpp

../src/common/adapter_internal.cpp.i:
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E $(curdir)/../src/common/adapter_internal.cpp > objects/adapter_internal.cpp.i

../src/common/adapter_internal.cpp.s:
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S $(curdir)/../src/common/adapter_internal.cpp -o objects/adapter_internal.cpp.s

../src/common/app_ble_gap.cpp.o: ../src/common/app_ble_gap.cpp
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o objects/app_ble_gap.cpp.o -c $(curdir)/../src/common/app_ble_gap.cpp

../src/common/app_ble_gap.cpp.i:
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E $(curdir)/../src/common/app_ble_gap.cpp > objects/app_ble_gap.cpp.i

../src/common/app_ble_gap.cpp.s:
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S $(curdir)/../src/common/app_ble_gap.cpp -o objects/app_ble_gap.cpp.s

../src/common/ble_common.cpp.o: ../src/common/ble_common.cpp
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o objects/ble_common.cpp.o -c $(curdir)/../src/common/ble_common.cpp

../src/common/ble_common.cpp.i:
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E $(curdir)/../src/common/ble_common.cpp > objects/ble_common.cpp.i

../src/common/ble_common.cpp.s:
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S $(curdir)/../src/common/ble_common.cpp -o objects/ble_common.cpp.s

../src/common/sd_rpc_impl.cpp.o: ../src/common/sd_rpc_impl.cpp
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o objects/sd_rpc_impl.cpp.o -c $(curdir)/../src/common/sd_rpc_impl.cpp

../src/common/sd_rpc_impl.cpp.i:
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E $(curdir)/../src/common/sd_rpc_impl.cpp > objects/sd_rpc_impl.cpp.i

../src/common/sd_rpc_impl.cpp.s:
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S $(curdir)/../src/common/sd_rpc_impl.cpp -o objects/sd_rpc_impl.cpp.s

../src/sd_api_v6/ble_gap_impl.cpp.o: ../src/sd_api_v6/ble_gap_impl.cpp
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o objects/ble_gap_impl.cpp.o -c $(curdir)/../src/sd_api_v6/ble_gap_impl.cpp

../src/sd_api_v6/ble_gap_impl.cpp.i:
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E $(curdir)/../src/sd_api_v6/ble_gap_impl.cpp > objects/ble_gap_impl.cpp.i

../src/sd_api_v6/ble_gap_impl.cpp.s:
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S $(curdir)/../src/sd_api_v6/ble_gap_impl.cpp -o objects/ble_gap_impl.cpp.s

../src/sd_api_v6/ble_gattc_impl.cpp.o: ../src/sd_api_v6/ble_gattc_impl.cpp
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o objects/ble_gattc_impl.cpp.o -c $(curdir)/../src/sd_api_v6/ble_gattc_impl.cpp

../src/sd_api_v6/ble_gattc_impl.cpp.i:
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E $(curdir)/../src/sd_api_v6/ble_gattc_impl.cpp > objects/ble_gattc_impl.cpp.i

../src/sd_api_v6/ble_gattc_impl.cpp.s:
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S $(curdir)/../src/sd_api_v6/ble_gattc_impl.cpp -o objects/ble_gattc_impl.cpp.s

../src/sd_api_v6/ble_gatts_impl.cpp.o: ../src/sd_api_v6/ble_gatts_impl.cpp
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o objects/ble_gatts_impl.cpp.o -c $(curdir)/../src/sd_api_v6/ble_gatts_impl.cpp

../src/sd_api_v6/ble_gatts_impl.cpp.i:
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E $(curdir)/../src/sd_api_v6/ble_gatts_impl.cpp > objects/ble_gatts_impl.cpp.i

../src/sd_api_v6/ble_gatts_impl.cpp.s:
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S $(curdir)/../src/sd_api_v6/ble_gatts_impl.cpp -o objects/ble_gatts_impl.cpp.s

../src/sd_api_v6/ble_impl.cpp.o: ../src/sd_api_v6/ble_impl.cpp
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o objects/ble_impl.cpp.o -c $(curdir)/../src/sd_api_v6/ble_impl.cpp

../src/sd_api_v6/ble_impl.cpp.i:
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E $(curdir)/../src/sd_api_v6/ble_impl.cpp > objects/ble_impl.cpp.i

../src/sd_api_v6/ble_impl.cpp.s:
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S $(curdir)/../src/sd_api_v6/ble_impl.cpp -o objects/ble_impl.cpp.s

../src/common/transport/h5.cpp.o: ../src/common/transport/h5.cpp
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o objects/h5.cpp.o -c $(curdir)/../src/common/transport/h5.cpp

../src/common/transport/h5.cpp.i:
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E $(curdir)/../src/common/transport/h5.cpp > objects/h5.cpp.i

../src/common/transport/h5.cpp.s:
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S $(curdir)/../src/common/transport/h5.cpp -o objects/h5.cpp.s

../src/common/transport/h5_transport.cpp.o: ../src/common/transport/h5_transport.cpp
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o objects/h5_transport.cpp.o -c $(curdir)/../src/common/transport/h5_transport.cpp

../src/common/transport/h5_transport.cpp.i:
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E $(curdir)/../src/common/transport/h5_transport.cpp > objects/h5_transport.cpp.i

../src/common/transport/h5_transport.cpp.s:
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S $(curdir)/../src/common/transport/h5_transport.cpp -o objects/h5_transport.cpp.s

../src/common/transport/serialization_transport.cpp.o: ../src/common/transport/serialization_transport.cpp
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o objects/serialization_transport.cpp.o -c $(curdir)/../src/common/transport/serialization_transport.cpp

../src/common/transport/serialization_transport.cpp.i:
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E $(curdir)/../src/common/transport/serialization_transport.cpp > objects/serialization_transport.cpp.i

../src/common/transport/serialization_transport.cpp.s:
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S $(curdir)/../src/common/transport/serialization_transport.cpp -o objects/serialization_transport.cpp.s

../src/common/transport/slip.cpp.o: ../src/common/transport/slip.cpp
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o objects/slip.cpp.o -c $(curdir)/../src/common/transport/slip.cpp

../src/common/transport/slip.cpp.i:
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E $(curdir)/../src/common/transport/slip.cpp > objects/slip.cpp.i

../src/common/transport/slip.cpp.s:
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S $(curdir)/../src/common/transport/slip.cpp -o objects/slip.cpp.s

../src/common/transport/transport.cpp.o: ../src/common/transport/transport.cpp
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o objects/transport.cpp.o -c $(curdir)/../src/common/transport/transport.cpp

../src/common/transport/transport.cpp.i:
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E $(curdir)/../src/common/transport/transport.cpp > objects/transport.cpp.i

../src/common/transport/transport.cpp.s:
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S $(curdir)/../src/common/transport/transport.cpp -o objects/transport.cpp.s

../src/common/transport/uart_settings.cpp.o: ../src/common/transport/uart_settings.cpp
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o objects/uart_settings.cpp.o -c $(curdir)/../src/common/transport/uart_settings.cpp

../src/common/transport/uart_settings.cpp.i:
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E $(curdir)/../src/common/transport/uart_settings.cpp > objects/uart_settings.cpp.i

../src/common/transport/uart_settings.cpp.s:
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S $(curdir)/../src/common/transport/uart_settings.cpp -o objects/uart_settings.cpp.s

../src/common/transport/uart_settings_boost.cpp.o: ../src/common/transport/uart_settings_boost.cpp
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o objects/uart_settings_boost.cpp.o -c $(curdir)/../src/common/transport/uart_settings_boost.cpp

../src/common/transport/uart_settings_boost.cpp.i:
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E $(curdir)/../src/common/transport/uart_settings_boost.cpp > objects/uart_settings_boost.cpp.i

../src/common/transport/uart_settings_boost.cpp.s:
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S $(curdir)/../src/common/transport/uart_settings_boost.cpp -o objects/uart_settings_boost.cpp.s

../src/common/transport/uart_transport.cpp.o: ../src/common/transport/uart_transport.cpp
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o objects/uart_transport.cpp.o -c $(curdir)/../src/common/transport/uart_transport.cpp

../src/common/transport/uart_transport.cpp.i:
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E $(curdir)/../src/common/transport/uart_transport.cpp > objects/uart_transport.cpp.i

../src/common/transport/uart_transport.cpp.s:
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S $(curdir)/../src/common/transport/uart_transport.cpp -o objects/uart_transport.cpp.s

../src/common/platform/linux/serial_port_enum.cpp.o: ../src/common/platform/linux/serial_port_enum.cpp
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o objects/serial_port_enum.cpp.o -c $(curdir)/../src/common/platform/linux/serial_port_enum.cpp

../src/common/platform/linux/serial_port_enum.cpp.i:
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E $(curdir)/../src/common/platform/linux/serial_port_enum.cpp > objects/serial_port_enum.cpp.i

../src/common/platform/linux/serial_port_enum.cpp.s:
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S $(curdir)/../src/common/platform/linux/serial_port_enum.cpp -o objects/serial_port_enum.cpp.s

nrf_ble_driver_obj_sd_api_v6: ../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/app_ble_user_mem.c.o
nrf_ble_driver_obj_sd_api_v6: ../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_app.c.o
nrf_ble_driver_obj_sd_api_v6: ../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_event.c.o
nrf_ble_driver_obj_sd_api_v6: ../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_evt_app.c.o
nrf_ble_driver_obj_sd_api_v6: ../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_gap_app.c.o
nrf_ble_driver_obj_sd_api_v6: ../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_gap_evt_app.c.o
nrf_ble_driver_obj_sd_api_v6: ../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_gattc_app.c.o
nrf_ble_driver_obj_sd_api_v6: ../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_gattc_evt_app.c.o
nrf_ble_driver_obj_sd_api_v6: ../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_gatts_app.c.o
nrf_ble_driver_obj_sd_api_v6: ../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_gatts_evt_app.c.o
nrf_ble_driver_obj_sd_api_v6: ../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_l2cap_app.c.o
nrf_ble_driver_obj_sd_api_v6: ../src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_l2cap_evt_app.c.o
nrf_ble_driver_obj_sd_api_v6: ../src/sd_api_v6/sdk/components/serialization/application/codecs/common/conn_systemreset_app.c.o
nrf_ble_driver_obj_sd_api_v6: ../src/sd_api_common/sdk/components/serialization/common/ble_serialization.c.o
nrf_ble_driver_obj_sd_api_v6: ../src/sd_api_common/sdk/components/serialization/common/cond_field_serialization.c.o
nrf_ble_driver_obj_sd_api_v6: ../src/sd_api_common/sdk/components/serialization/common/struct_ser/ble/ble_gap_struct_serialization.c.o
nrf_ble_driver_obj_sd_api_v6: ../src/sd_api_common/sdk/components/serialization/common/struct_ser/ble/ble_gattc_struct_serialization.c.o
nrf_ble_driver_obj_sd_api_v6: ../src/sd_api_common/sdk/components/serialization/common/struct_ser/ble/ble_gatts_struct_serialization.c.o
nrf_ble_driver_obj_sd_api_v6: ../src/sd_api_common/sdk/components/serialization/common/struct_ser/ble/ble_gatt_struct_serialization.c.o
nrf_ble_driver_obj_sd_api_v6: ../src/sd_api_common/sdk/components/serialization/common/struct_ser/ble/ble_l2cap_struct_serialization.c.o
nrf_ble_driver_obj_sd_api_v6: ../src/sd_api_common/sdk/components/serialization/common/struct_ser/ble/ble_struct_serialization.c.o
nrf_ble_driver_obj_sd_api_v6: ../src/common/adapter_internal.cpp.o
nrf_ble_driver_obj_sd_api_v6: ../src/common/app_ble_gap.cpp.o
nrf_ble_driver_obj_sd_api_v6: ../src/common/ble_common.cpp.o
nrf_ble_driver_obj_sd_api_v6: ../src/common/sd_rpc_impl.cpp.o
nrf_ble_driver_obj_sd_api_v6: ../src/sd_api_v6/ble_gap_impl.cpp.o
nrf_ble_driver_obj_sd_api_v6: ../src/sd_api_v6/ble_gattc_impl.cpp.o
nrf_ble_driver_obj_sd_api_v6: ../src/sd_api_v6/ble_gatts_impl.cpp.o
nrf_ble_driver_obj_sd_api_v6: ../src/sd_api_v6/ble_impl.cpp.o
nrf_ble_driver_obj_sd_api_v6: ../src/common/transport/h5.cpp.o
nrf_ble_driver_obj_sd_api_v6: ../src/common/transport/h5_transport.cpp.o
nrf_ble_driver_obj_sd_api_v6: ../src/common/transport/serialization_transport.cpp.o
nrf_ble_driver_obj_sd_api_v6: ../src/common/transport/slip.cpp.o
nrf_ble_driver_obj_sd_api_v6: ../src/common/transport/transport.cpp.o
nrf_ble_driver_obj_sd_api_v6: ../src/common/transport/uart_settings.cpp.o
nrf_ble_driver_obj_sd_api_v6: ../src/common/transport/uart_settings_boost.cpp.o
nrf_ble_driver_obj_sd_api_v6: ../src/common/transport/uart_transport.cpp.o
nrf_ble_driver_obj_sd_api_v6: ../src/common/platform/linux/serial_port_enum.cpp.o
# nrf_ble_driver_obj_sd_api_v6: build.make

.PHONY : nrf_ble_driver_obj_sd_api_v6

# Rule to build all files generated by this target.
build: nrf_ble_driver_obj_sd_api_v6

.PHONY : build
