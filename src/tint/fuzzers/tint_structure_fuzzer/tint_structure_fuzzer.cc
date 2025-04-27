#include <algorithm>
#include <cassert>
#include <exception>
#include <iostream>
#include <iterator>
#include <numeric>
#include <type_traits>
#include <vector>
#include "src/tint/cmd/fuzz/wgsl/fuzz.h"
#include "src/tint/fuzzers/random_generator.h"
#include "src/tint/fuzzers/tint_common_fuzzer.h"
#include "src/tint/fuzzers/transform_builder.h"
#include "src/tint/lang/wgsl/ast/transform/add_empty_entry_point.h"
#include "src/tint/lang/wgsl/ast/transform/builtin_polyfill.h"
#include "src/tint/lang/wgsl/ast/transform/fold_constants.h"
#include "src/tint/lang/wgsl/ast/transform/remove_unreachable_statements.h"
#include "src/tint/lang/wgsl/ast/transform/renamer.h"
#include "src/tint/lang/wgsl/ast/transform/unshadow.h"
#include "src/tint/lang/wgsl/reader/reader.h"
#include "src/tint/lang/wgsl/writer/writer.h"
#include "src/tint/utils/cli/cli.h"
#include "testing/libfuzzer/libfuzzer_exports.h"

#include "probabilities.h"
#include "syntax.h"

namespace {

tint::fuzz::wgsl::Options options;

}  // namespace

using namespace std::string_view_literals;

namespace tint::fuzzers::structure_fuzzer {
namespace {

static std::optional<Probabilities> mutations;

static void printHex(FILE* f, const uint8_t* data, size_t size) {
    for (size_t i = 0; i < size; ++i) {
        fprintf(f, "%02X", data[i]);
    }
    fprintf(f, "\n");
}

extern "C" int LLVMFuzzerInitialize(int* argc, char*** argv) {
    tint::cli::OptionSet opts;

    constexpr size_t numMutations = static_cast<int>(Mutation::Last) + 1;

    std::vector<unsigned> list;
    for (int i = 1; i < *argc; ++i) { 
        std::string_view arg((*argv)[i]);
        if (arg.find("--prob=") == 0) {
            arg.remove_prefix(7);
            size_t start = 0;
            size_t stop = arg.find_first_of(", ", start);
            while (start != std::string_view::npos) {
                auto result = strconv::ParseNumber<unsigned>(arg.substr(start, stop - start));
                assert(result == tint::Success);
                list.push_back(result.Get());
                if (stop == std::string_view::npos)
                    break;
                start = stop + 1;
                stop = arg.find_first_of(", ", start);
            }
        }
    }
    fprintf(stdout, "Mutation parameters:\n");
    for (int i = 0; i < list.size(); ++i) {
        fprintf(stdout, "[%d] %u\n", i, list[i]);
    }

    if (list.size() != numMutations) {
        fprintf(stderr, "Incorrect number of arguments. Expected %zu\n", numMutations);
        list.resize(numMutations, 10);
    }
    mutations.emplace(std::move(list));

#if 0
    {
        RandomGenerator gen{1};
        std::string wgsl_str;

        std::vector<uint8_t> input{};
        for (int i = 0; i < 1000; ++i) {
            Mutation mutation = mutations->sample<Mutation>(gen);
            wgsl_str = WGSLSource(input.data(), input.size());
            printHex(stderr, input.data(), input.size());
            fprintf(stderr, "%s\n\n", wgsl_str.c_str());
            input = WGSLMutate(mutation, input.data(), input.size(), gen);
        }
        std::exit(1);
    }
#endif
    return 0;
}

extern "C" size_t LLVMFuzzerCustomMutator(uint8_t* data,
                                          size_t size,
                                          size_t maxSize,
                                          unsigned int seed) {
    if (!mutations.has_value()) {
        return 0;
    }
    RandomGenerator gen(seed);
    Mutation mutation = mutations->sample<Mutation>(gen);
    std::vector<uint8_t> output = WGSLMutate(mutation, data, size, gen);
    if (output.size() > maxSize) {
        fprintf(stderr, "Mutate output truncated %zu -> %zu\n", output.size(), maxSize);
        fflush(stderr);
        output.resize(maxSize);
    }
    memcpy(data, output.data(), output.size());
#if 0
    static FILE* fdbg = nullptr;
    if (!fdbg) {
        fdbg = fopen("fuzzer-mutator.txt", "w");
    }
    fprintf(fdbg, "@@@ Mutate m=%d seed=%u\n", (int)mutation, seed);
    fprintf(fdbg, "IN: ");
    printHex(fdbg, data, size);
    std::string wgsl_str = WGSLSource(data, size);
    fprintf(fdbg, "IN: %s\n", wgsl_str.c_str());
    fprintf(fdbg, "OUT: ");
    printHex(fdbg, output.data(), output.size());
    wgsl_str = WGSLSource(output.data(), output.size());
    fprintf(fdbg, "OUT: %s\n", wgsl_str.c_str());
    fflush(fdbg);
#endif
    return output.size();
}

static FILE* dbg[2]{nullptr, nullptr};

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    if (size <= 1) {
        return 0;
    }
    std::string wgsl_str = WGSLSource(data, size);

    static const bool debug = std::getenv("TINT_STRUCTURE_FUZZER_DEBUG");

    if (debug && !dbg[0]) {
        dbg[0] = fopen("fuzzer-failure.txt", "w");
        dbg[1] = fopen("fuzzer-success.txt", "w");
    }

    bool successfull = false;

    for (OutputFormat fmt :
         {OutputFormat::kWGSL, OutputFormat::kSpv, OutputFormat::kHLSL, OutputFormat::kMSL}) {
        TransformBuilder tb(data, size);

        tb.AddTransform<tint::ast::transform::Renamer>();
        tb.AddTransform<tint::ast::transform::Robustness>();
        tb.manager()->Add<tint::ast::transform::FoldConstants>();
        tb.manager()->Add<tint::ast::transform::BuiltinPolyfill>();
        tb.manager()->Add<tint::ast::transform::AddEmptyEntryPoint>();
        tb.manager()->Add<tint::ast::transform::Unshadow>();

        CommonFuzzer fuzzer(InputFormat::kWGSL, fmt);
        fuzzer.SetTransformManager(tb.manager(), tb.data_map());

        fuzzer.Run(reinterpret_cast<const uint8_t*>(wgsl_str.data()), wgsl_str.size());
        if (debug && fmt == OutputFormat::kSpv) {
            if (!fuzzer.HasErrors()) {
                successfull = true;
                fprintf(dbg[1], "|IN(%zu): ", size);
                printHex(dbg[1], data, size);
                fprintf(dbg[1], " OUT: %s\n", wgsl_str.c_str());
               
            } else {
                fprintf(dbg[0], "|IN(%zu): ", size);
                printHex(dbg[0], data, size);
                fprintf(dbg[0], " OUT: %s\n", wgsl_str.c_str());
                fprintf(dbg[0], " ERROR: %s", fuzzer.Diagnostics().Str().c_str());
            
            }
        }
    }
    return 0; 
}
}  // namespace
}  // namespace tint::fuzzers::structure_fuzzer

