all:
	-rm -rf build
	mkdir build
	cd build ; \
	qmake ../SonyTOF_client.pro ; \
	make -j$(nproc) 
	@echo "SonyTOF_client application built at $(shell pwd)/build/SonyTOF_client"
