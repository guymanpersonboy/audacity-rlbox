.PHONY: resample_sandboxed

RESAMPLE_HEADER_PATH:=../
UTILITY_HEADER_PATH:=../../lib-utility
COMPONENTS_HEADER_PATH:=../../lib-components
PREFEERENCES_HEADER_PATH:=../../lib-preferences
STRINGS_HEADER_PATH:=../../lib-strings
RLBOX_HEADER_PATH:=../../../../include/rlbox
INTEGRATION_HEADER_PATH:=../../../../include/wasm_sandbox
CXXFLAGS:=-g -Wall
INCLUDE_FLAGS:=-I $(RLBOX_HEADER_PATH) -I $(RESAMPLE_HEADER_PATH)	     \
				-I $(UTILITY_HEADER_PATH) -I $(COMPONENTS_HEADER_PATH)   \
				-I $(PREFEERENCES_HEADER_PATH) -I $(STRINGS_HEADER_PATH) \
				-I $(INTEGRATION_HEADER_PATH)

#Will not build until you sandbox the library
resample_sandboxed: ResampleSandboxed.cpp ResampleSandboxed.h
	$(CXX) -std=c++17 $(CXXFLAGS) $(INCLUDE_FLAGS) -pthread ResampleSandboxed.cpp -ldl -lrt -o $@

clean:
	-rm -f ResampleSandboxed
