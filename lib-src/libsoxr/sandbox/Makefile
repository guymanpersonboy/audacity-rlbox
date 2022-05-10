CC=build/_deps/wasiclang-src/bin/clang                           
CXX=build/_deps/wasiclang-src/bin/clang++                        
CFLAGS="--sysroot build/_deps/wasiclang-src/share/wasi-sysroot"  
LD=build/_deps/wasiclang-src/bin/wasm-ld                         
LDLIBS=wasm2c_sandbox_wrapper.o                                  
LDFLAGS="-Wl,--export-all -Wl,--no-entry -Wl,--growable-table"   

wrapper:
	build/_deps/wasiclang-src/bin/clang --sysroot build/_deps/wasiclang-src/share/wasi-sysroot wasm2c_sandbox_wrapper.c -c -o wasm2c_sandbox_wrapper.o

wasm:
	$(CC) $(CFLAGS) $(LDFLAGS) wasm2c_sandbox_wrapper.c ../src/soxr.c -o libSoX.wasm

c:
	build/_deps/mod_wasm2c-src/bin/wasm2c -o libWasmSoX.c libSoX.wasm

so:
	gcc -shared -fPIC -O3 -o libWasmSoX.so libWasmSoX.c build/_deps/mod_wasm2c-src/wasm2c/wasm-rt-impl.c build/_deps/mod_wasm2c-src/wasm2c/wasm-rt-os-unix.c build/_deps/mod_wasm2c-src/wasm2c/wasm-rt-os-win.c build/_deps/mod_wasm2c-src/wasm2c/wasm-rt-wasi.c
