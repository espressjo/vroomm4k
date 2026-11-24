mkdir:
	@mkdir -p /opt/vroomm4k/config
	@mkdir -p /opt/vroomm4k/log
	@mkdir -p /opt/vroomm4k/data
	@mkdir -p /opt/vroomm4k/bin
	@mkdir -p /opt/vroomm4k/include

all:
	@cd ./src && make all
clean:
	@cd ./src && make clean
install: mkdir
	@cd ./config && make install
	@cd ./src && make install

