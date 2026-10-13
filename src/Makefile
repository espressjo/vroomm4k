CC = g++
CPPFLAGS +=
TARGET = vroomm4k

BASEPATH = /opt/vroomm4k

INCL = -I./ \
	-I/usr/local/include

LIB = 	-L/usr/local/lib -luics \
				-lpthread -llabdev -lsoem -lomm\
				-lprometheus-cpp-core -lprometheus-cpp-pull
RPATH_FLAGS = -Wl,--disable-new-dtags \
	-Wl,-rpath,/usr/local/lib \
              -Wl,-rpath,/usr/local/lib/uics


LDFLAGS += $(RPATH_FLAGS)
CFLAGS = -std=c++17 -g

%.o: %.cpp
	$(CC) $(CFLAGS) $(CPPFLAGS) $(INCL) -c $< -o $@


OBJS =	main.o \
				vroomm4k_getstatus.o \
				vroomm4k_devices.o \
				devManagerCtrl.o

all: $(OBJS)
	$(CC)  $(CPPFLAGS) $(CFLAGS) $(OBJS) $(LDFLAGS) -o $(TARGET) $(INCL) $(LIB)
clean:
	@rm -f $(TARGET) $(OBJS)

install:
	@cp vroomm4k_telemetry_t.h $(BASEPATH)/include/
	@cp insthandle.h $(BASEPATH)/include/
	@cp $(TARGET) $(BASEPATH)/bin/$(TARGET)
	@sudo setcap cap_net_raw,cap_net_admin+eip $(BASEPATH)/bin/$(TARGET)
