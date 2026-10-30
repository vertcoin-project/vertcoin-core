# Copyright (c) 2023-present The Bitcoin Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or https://opensource.org/license/mit/.

add_library(bitcoin_crypto STATIC EXCLUDE_FROM_ALL
  $<$<NOT:$<STREQUAL:${CMAKE_SYSTEM_NAME},Generic>>:aes.cpp>
  chacha20.cpp
  chacha20poly1305.cpp
  hex_base.cpp
  hkdf_sha256_32.cpp
  hmac_sha256.cpp
  hmac_sha512.cpp
  Lyra2RE/Lyra2RE.c
  Lyra2RE/Lyra2.c
  Lyra2RE/Sponge.c
  Lyra2RE/blake.c
  Lyra2RE/groestl.c
  Lyra2RE/keccak.c
  Lyra2RE/skein.c
  Lyra2RE/cubehash.c
  Lyra2RE/bmw.c
  muhash.cpp
  poly1305.cpp
  ripemd160.cpp
  scrypt.cpp
  sha1.cpp
  sha256.cpp
  sha256_sse4.cpp
  sha3.cpp
  sha512.cpp
  siphash.cpp
  tiny_sha3/sha3.c
  ../support/cleanse.cpp
  $<$<NOT:$<STREQUAL:${CMAKE_SYSTEM_NAME},Generic>>:../support/lockedpool.cpp>
)

# The sph hash code type-puns through the sph_*_aligned helpers; GCC 15 at -O2
# miscompiles bmw.c under strict aliasing, breaking Lyra2REv2/v3.
# These are vendored third-party sources, so their warnings (tabs, -Wundef,
# -Wsign-compare) are suppressed with -w, as upstream does for its vendored code.
set_source_files_properties(Lyra2RE/Lyra2RE.c Lyra2RE/Lyra2.c Lyra2RE/Sponge.c Lyra2RE/blake.c Lyra2RE/groestl.c Lyra2RE/keccak.c Lyra2RE/skein.c Lyra2RE/cubehash.c Lyra2RE/bmw.c tiny_sha3/sha3.c PROPERTIES COMPILE_OPTIONS "$<$<NOT:$<C_COMPILER_ID:MSVC>>:-fno-strict-aliasing;-w>")
# Vendored scrypt code (VLA, tabs, -Wclass-memaccess): suppress its warnings.
set_source_files_properties(scrypt.cpp PROPERTIES COMPILE_OPTIONS "$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-w>")

target_link_libraries(bitcoin_crypto
  PRIVATE
    core_interface
)

if(HAVE_SSE41)
  target_compile_definitions(bitcoin_crypto PRIVATE ENABLE_SSE41)
  target_sources(bitcoin_crypto PRIVATE sha256_sse41.cpp)
  set_property(SOURCE sha256_sse41.cpp PROPERTY
    COMPILE_OPTIONS ${SSE41_CXXFLAGS}
  )
endif()

if(HAVE_AVX2)
  target_compile_definitions(bitcoin_crypto PRIVATE ENABLE_AVX2)
  target_sources(bitcoin_crypto PRIVATE sha256_avx2.cpp)
  set_property(SOURCE sha256_avx2.cpp PROPERTY
    COMPILE_OPTIONS ${AVX2_CXXFLAGS}
  )
endif()

if(HAVE_SSE41 AND HAVE_X86_SHANI)
  target_compile_definitions(bitcoin_crypto PRIVATE ENABLE_SSE41 ENABLE_X86_SHANI)
  target_sources(bitcoin_crypto PRIVATE sha256_x86_shani.cpp)
  set_property(SOURCE sha256_x86_shani.cpp PROPERTY
    COMPILE_OPTIONS ${SSE41_CXXFLAGS} ${X86_SHANI_CXXFLAGS}
  )
endif()

if(HAVE_ARM_SHANI)
  target_compile_definitions(bitcoin_crypto PRIVATE ENABLE_ARM_SHANI)
  target_sources(bitcoin_crypto PRIVATE sha256_arm_shani.cpp)
  set_property(SOURCE sha256_arm_shani.cpp PROPERTY
    COMPILE_OPTIONS ${ARM_SHANI_CXXFLAGS}
  )
endif()
