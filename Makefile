CPP=g++
CPPFLAGS= -I/usr/local/include/modbus  -std=c++11
#CPP=arm-linux-g++
#CPPFLAGS= -I./include/modbus -L./lib -std=c++11
LIBS=-lmodbus -lpthread
server_src = ModbusServer.cpp Server.cpp
client_src = Client.cpp

server_obj = $(patsubst %.cpp, %.o, $(server_src))
client_obj = $(patsubst %.cpp, %.o, $(client_src))

server = svr
client = cli
target = $(server) $(client)

ALL:$(target)


$(server_obj):%.o:%.cpp
	$(CPP) -c $< -o $@ $(CPPFLAGS) $(LIBS)

$(client_obj):%.o:%.cpp
	$(CPP) -c $< -o $@ $(CPPFLAGS) $(LIBS)


$(server): $(server_obj)
	$(CPP) $^ -o $@ $(CPPFLAGS) $(LIBS)

$(client): $(client_obj)
	$(CPP) $^ -o $@ $(CPPFLAGS) $(LIBS)


clean:
	@-rm -rf $(server_obj) $(client_obj) $(target)

distclean:
	@-rm -rf $(target)

.PHONY: clean ALL distclean
