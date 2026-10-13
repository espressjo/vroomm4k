PREFIX = /opt/vroomm4k
folder:
	@mkdir -p $(PREFIX)/config
	@mkdir -p $(PREFIX)/data
	@mkdir -p $(PREFIX)/log
install:
	@echo "Installing UICS configuration file"
	@cp cmd.cfg $(PREFIX)/config/cmd.conf
	@cp state.cfg $(PREFIX)/config/state.conf
	@cp uics.cfg $(PREFIX)/config/uics.conf
	@cp vroomm4k.cfg $(PREFIX)/config/vroomm4k.conf
	@cp lakeshore.cfg $(PREFIX)/config/lakeshore.cfg
	@cp sunpower.cfg $(PREFIX)/config/sunpower.cfg
	@cp mks972b.cfg $(PREFIX)/config/mks972b.cfg

