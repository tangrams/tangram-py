default: all

clean: clean_lib clean_wrap

clean_lib: 
	rm -rf tangram/build
	rm -f tangram/libtangram*.a

clean_wrap:
	rm -f tangram/_tangram.so
	rm -f tangram/tangram.py
	rm -f tangram/tangram_wrap.cpp
	rm -rf build

all:
	cd tangram/ && ./make.sh && cd ..
	python setup.py build_ext --inplace 