ifndef PORT
PORT=/dev/ttyUSB0
endif
BOARD=esp8266:esp8266:nodemcuv2
XTAL=:xtal=160
EESZ=,eesz=4M1M

ESPTOOL=$(wildcard $(HOME)/.arduino15/packages/esp8266/hardware/esp8266/*/tools/esptool/esptool.py)
BASICOTA=$(wildcard $(HOME)/.arduino15/packages/esp8266/hardware/esp8266/*/tools/espota.py)


SRC = $(wildcard *.ino)
PROJECT = $(notdir $(CURDIR))
TARGET = $(PROJECT).ino.bin
FS_SRC= $(wildcard html/*)

$(TARGET): $(SRC) html.h version.h
	@rm -rf tmp
	@mkdir -p tmp
	TMPDIR=$(PWD)/tmp arduino-cli compile --fqbn=$(BOARD)$(XTAL)$(EESZ) --output-dir=$(PWD)
	@rm -rf tmp

html.h: $(FS_SRC)
	./gen_html > $@

recompile: $(TARGET)

netupload: $(TARGET)
ifdef host
	python3 $(BASICOTA) -i ${host} -P 8266 -d -f $(TARGET) ${pass}
else
	@echo Need host=target to be set - eg make $@ host=testesp
	@echo 'If a password is set then also supply pass="-a password"'
endif

upload:
	@mkdir -p tmp
	TMPDIR=$(PWD)/tmp arduino-cli upload --fqbn=$(BOARD) -p $(PORT) --input-dir=$(PWD)
	@rm -rf tmp

##	python $(ESPTOOL) --port=$(PORT) write_flash 0x0 $(TARGET)

serial:
	@kermit -l $(PORT) -b 115200 -c

clean:
	rm -rf *.elf tmp *.bin html.h *.map
