cmake_minimum_required(VERSION 3.22.0)
project(olm VERSION 3.2.16 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 23)

set(OLM_SOURCES
    src/account.cpp
    src/base64.cpp
    src/cipher.cpp
    src/crypto.cpp
    src/error.cpp
    src/inbound_group_session.cpp
    src/memory.cpp
    src/message.cpp
    src/olm.cpp
    src/outbound_group_session.cpp
    src/pickle.cpp
    src/pk.cpp
    src/ratchet.cpp
    src/sas.cpp
    src/session.cpp
    src/utility.cpp
)

set(OLM_LIB_SOURCES
    # Ed25519
    lib/ed25519/src/fe_0.c
    lib/ed25519/src/fe_1.c
    lib/ed25519/src/fe_add.c
    lib/ed25519/src/fe_cmov.c
    lib/ed25519/src/fe_copy.c
    lib/ed25519/src/fe_frombytes.c
    lib/ed25519/src/fe_invert.c
    lib/ed25519/src/fe_isnegative.c
    lib/ed25519/src/fe_isnonzero.c
    lib/ed25519/src/fe_mul.c
    lib/ed25519/src/fe_neg.c
    lib/ed25519/src/fe_pow22523.c
    lib/ed25519/src/fe_sq.c
    lib/ed25519/src/fe_sq2.c
    lib/ed25519/src/fe_sub.c
    lib/ed25519/src/fe_tobytes.c
    lib/ed25519/src/ge_add.c
    lib/ed25519/src/ge_double_scalarmult.c
    lib/ed25519/src/ge_frombytes.c
    lib/ed25519/src/ge_madd.c
    lib/ed25519/src/ge_msub.c
    lib/ed25519/src/ge_p1p1_to_p2.c
    lib/ed25519/src/ge_p1p1_to_p3.c
    lib/ed25519/src/ge_p2_0.c
    lib/ed25519/src/ge_p2_dbl.c
    lib/ed25519/src/ge_p3_0.c
    lib/ed25519/src/ge_p3_dbl.c
    lib/ed25519/src/ge_p3_to_cached.c
    lib/ed25519/src/ge_p3_to_p2.c
    lib/ed25519/src/ge_p3_tobytes.c
    lib/ed25519/src/ge_precomp_0.c
    lib/ed25519/src/ge_scalarmult_base.c
    lib/ed25519/src/ge_sub.c
    lib/ed25519/src/ge_tobytes.c
    lib/ed25519/src/keypair.c
    lib/ed25519/src/sc_muladd.c
    lib/ed25519/src/sc_reduce.c
    lib/ed25519/src/sign.c
    lib/ed25519/src/verify.c
    # Curve25519-donna
    lib/curve25519-donna/curve25519-donna.c
    # Crypto algorithms
    lib/crypto-algorithms/aes.c
    lib/crypto-algorithms/sha256.c
    lib/crypto-algorithms/sha512.c
)

add_library(olm STATIC ${OLM_SOURCES} ${OLM_LIB_SOURCES})

target_include_directories(olm
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/ed25519/src
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/curve25519-donna
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/crypto-algorithms
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/doctest/doctest
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_definitions(olm PRIVATE OLM_EXPORT= OLM_STATIC_DEFINE=1)

if(NOT MSVC)
    target_compile_options(olm PRIVATE -Wall -Wextra -Wno-unused-parameter)
endif()
