
# requirements

if ! which git >/dev/null ; then
    echo ERROR: git not found
    exit 1
fi

if ! which cmake >/dev/null ; then
    echo ERROR: cmake not found
    exit 1
fi

# get depot tools
if [[ ! -d depot_tools ]] ; then
    git clone --depth=1 --no-tags --single-branch https://chromium.googlesource.com/chromium/tools/depot_tools.git || exit 1
fi

export PATH=$(pwd)/depot_tools:$PATH

# clone dawn

DAWN_COMMIT=b7a1641725fa62df9337bc4975a12a94abe1f00d # hardcoded commit for now

if [[ ! -d dawn ]] ; then
    git clone --no-tags --single-branch https://dawn.googlesource.com/dawn || exit 1
else
    cd dawn
    git restore src/dawn/native/CMakeLists.txt
    git pull --force --no-tags || exit 1
    git checkout $DAWN_COMMIT || exit 1
    cd ..
fi

cd dawn
cp scripts/standalone.gclient .gclient
gclient sync || exit 1
cd ..

cat >> dawn/src/dawn/native/CMakeLists.txt <<EOF

add_library(webgpu SHARED ${DAWN_PLACEHOLDER_FILE})
common_compile_options(webgpu)
target_link_libraries(webgpu PRIVATE dawn_native)
target_link_libraries(webgpu PUBLIC dawn_headers)
target_compile_definitions(webgpu PRIVATE WGPU_IMPLEMENTATION WGPU_SHARED_LIBRARY)
target_sources(webgpu PRIVATE ${WEBGPU_DAWN_NATIVE_PROC_GEN})

EOF

# build dawn

cmake                                         \
  -S dawn                                     \
  -B dawn.build                               \
  -D CMAKE_BUILD_TYPE=Debug                   \
  -D CMAKE_POLICY_DEFAULT_CMP0091=NEW         \
  -D BUILD_SHARED_LIBS=OFF                    \
  -D BUILD_SAMPLES=OFF                        \
  -D DAWN_ENABLE_METAL=ON                     \
  -D DAWN_ENABLE_NULL=OFF                     \
  -D DAWN_ENABLE_DESKTOP_GL=OFF               \
  -D DAWN_ENABLE_OPENGLES=OFF                 \
  -D DAWN_ENABLE_VULKAN=OFF                   \
  -D DAWN_BUILD_SAMPLES=OFF                   \
  -D TINT_BUILD_SAMPLES=OFF                   \
  -D TINT_BUILD_DOCS=OFF                      \
  -D TINT_BUILD_TESTS=OFF                     \
  || exit 1

cmake --build dawn.build --config Debug --target webgpu --parallel


# package result

if [ -d dawn.out ] ; then
    rm -r dawn.out
fi

mkdir dawn.out
mkdir dawn.out/bin
mkdir dawn.out/include

cp dawn.build/gen/include/dawn/webgpu.h dawn.out/include/
cp dawn.build/src/dawn/native/libwebgpu.dylib dawn.out/bin/
