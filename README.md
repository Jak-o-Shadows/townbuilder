# townbuilder


conan profile detect --force

conan install . --output-folder=build --build=missing

cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -G "Visual Studio 16 2019"

cmake --build . --config Release