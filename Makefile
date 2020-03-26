name = Fizzbuzz
xml_report_file="test_detail.xml"
text_error_file="error.txt"

MAKEFLAGS += --silent

all: resources tests

resources:
	if [ ! -d "lib" ]; then mkdir lib; fi
	if [ ! -d "build" ]; then mkdir build; fi
	cd lib; if [ ! -d "googletest" ] ; then git clone -b 'master' --single-branch --depth 1 https://github.com/google/googletest; fi	
	cd build; cmake .. -DCMAKE_BUILD_TYPE=Debug -G "Unix Makefiles";

tests:
	- @cd build; cmake .. -DCMAKE_BUILD_TYPE=Debug -G "Unix Makefiles" > /dev/null; make 2> test/${text_error_file} 1> /dev/null && \
	cd test; ./${name}_test --gtest_eh_output 2> ${text_error_file} 1>/dev/null;

	@cd build/test; \
	if [ -e ${xml_report_file} ]; \
	then cat ${xml_report_file}; rm ${xml_report_file};\
	else cat ${text_error_file}; rm ${text_error_file}; \
	fi
