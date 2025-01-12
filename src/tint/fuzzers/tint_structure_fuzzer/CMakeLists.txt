
add_executable(tint_structure_fuzzer
    tint_structure_fuzzer.cc
    syntax.cc
    syntax.h
    probabilities.h
    ../mersenne_twister_engine.cc
    ../mersenne_twister_engine.h
    ../random_generator.cc
    ../random_generator.h
    ../tint_common_fuzzer.cc
    ../tint_common_fuzzer.h
    ../random_generator_engine.cc
    ../random_generator_engine.h
)

if (NOT WIN32)
    set_source_files_properties(syntax.cc PROPERTIES COMPILE_FLAGS -O1)
endif()

tint_fuzzer_compile_options(tint_structure_fuzzer)
tint_spvtools_compile_options(tint_structure_fuzzer)
target_include_directories(tint_structure_fuzzer PRIVATE ${CMAKE_BINARY_DIR})
target_link_libraries(tint_structure_fuzzer PRIVATE tint_lang_hlsl_writer_helpers)
target_link_libraries(tint_structure_fuzzer PRIVATE tint_lang_msl_writer_helpers)
target_link_libraries(tint_structure_fuzzer PRIVATE tint_lang_spirv_writer_helpers)

tint_core_compile_options(tint_structure_fuzzer)

if (TINT_STRUCTURE_FUZZER_SANITIZERS)
    target_compile_options(tint_structure_fuzzer PRIVATE -fsanitize=address -fsanitize=undefined)
    target_link_options(tint_structure_fuzzer PRIVATE -fsanitize=address -fsanitize=undefined)
endif ()
