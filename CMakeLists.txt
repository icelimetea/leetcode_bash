cmake_minimum_required(VERSION 3.9)

set(CMAKE_C_COMPILER "gcc")

project(leetcode_bash LANGUAGES C)

set(CMAKE_C_FLAGS "-nostdlib -Wall -Wextra -Werror -pedantic -fno-builtin")

set(CMAKE_C_FLAGS_RELEASE "-march=znver3 -mmmx -mpopcnt -msse -msse2 -msse3 -mssse3 -msse4.1 -msse4.2 -mavx -mavx2 -msse4a -mno-fma4 -mno-xop -mfma -mno-avx512f -mbmi -mbmi2 -maes -mpclmul -mno-avx512vl -mno-avx512bw -mno-avx512dq -mno-avx512cd -mno-avx512er -mno-avx512pf -mno-avx512vbmi -mno-avx512ifma -mno-avx5124vnniw -mno-avx5124fmaps -mno-avx512vpopcntdq -mno-avx512vbmi2 -mno-gfni -mvpclmulqdq -mno-avx512vnni -mno-avx512bitalg -mno-avx512bf16 -mno-avx512vp2intersect -mno-3dnow -madx -mabm -mno-cldemote -mclflushopt -mclwb -mclzero -mcx16 -mno-enqcmd -mf16c -mfsgsbase -mfxsr -mno-hle -msahf -mno-lwp -mlzcnt -mmovbe -mno-movdir64b -mno-movdiri -mno-mwaitx -mno-pconfig -mno-pku -mno-prefetchwt1 -mprfchw -mno-ptwrite -mrdpid -mrdrnd -mrdseed -mno-rtm -mno-serialize -mno-sgx -msha -mno-shstk -mno-tbm -mno-tsxldtrk -mvaes -mno-waitpkg -mno-wbnoinvd -mxsave -mxsavec -mxsaveopt -mno-xsaves -mno-amx-tile -mno-amx-int8 -mno-amx-bf16 -mno-uintr -mno-hreset -mno-kl -mno-widekl -mno-avxvnni --param l1-cache-size=32 --param l1-cache-line-size=64 --param l2-cache-size=512 -mtune=znver3 -Os -fno-asynchronous-unwind-tables")
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)

set(CMAKE_C_FLAGS_DEBUG "-O2 -ggdb")

add_link_options("$<$<CONFIG:Release>:LINKER:--strip-all,--nmagic,--orphan-handling=warn,-T,${CMAKE_SOURCE_DIR}/linker.ld,-z,nosectionheader>")

include_directories(include)

add_subdirectory(phone_numbers)
add_subdirectory(tenth_line)
add_subdirectory(transpose_file)
add_subdirectory(wordcount)
