# townbuilder


conan profile detect --force

conan install . --output-folder=build --build=missing

cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=conan_toolchain
