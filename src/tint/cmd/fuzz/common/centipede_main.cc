// Copyright 2026 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// GEN_BUILD:IGNORE_FILE

/**
 * This a custom Tint-owned wrapper/shim for building our libFuzzer
 * based fuzzers using Centipede in GN builds.
 *
 * This is used instead of `//third_party/fuzztest:llvm_wrapper`, because
 * as part of merging Centipede + FuzzTest, the upstream wrapper/shim
 * intentionally defines `LLVMFuzzerCustomCrossOver` to force a
 * duplicate-symbol link error against `libprotobuf-mutator`, which is
 * currently used by the IR fuzzers.
 *
 * Additionally there are lesser issues around differences in flag
 * parsing order and C++ versions (23 v 20) that could be mitigated
 * for the WGSL fuzzers, but since the custom implementation is needed
 * for the IR fuzzers, it makes more sense for all of the fuzzers to
 * use it.
 *
 * If/when Tint's fuzzers are migrated to FuzzTest,
 * crbug.com/524554521, this can go away.
 */

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "src/tint/utils/command/cli.h"
#include "src/tint/utils/macros/compiler.h"
#include "src/utils/compiler.h"

TINT_BEGIN_DISABLE_ALL_WARNINGS();
#include "absl/base/no_destructor.h"
#include "absl/base/thread_annotations.h"
#include "absl/random/bit_gen_ref.h"
#include "absl/random/random.h"
#include "absl/synchronization/mutex.h"
#include "common/logging.h"
#include "fuzztest/fuzztest_macros.h"
#include "fuzztest/init_fuzztest.h"
#include "fuzztest/internal/domains/arbitrary_impl.h"
#include "fuzztest/internal/domains/container_of_impl.h"
#include "fuzztest/internal/domains/domain_base.h"
#include "fuzztest/internal/domains/lazy.h"
#include "fuzztest/internal/io.h"
#include "gtest/gtest.h"
TINT_END_DISABLE_ALL_WARNINGS();

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size);

extern "C" int __attribute__((weak)) LLVMFuzzerInitialize(int* argc, char*** argv);

extern "C" size_t __attribute__((weak)) LLVMFuzzerCustomMutator(uint8_t* data,
                                                                size_t size,
                                                                size_t max_size,
                                                                unsigned int seed);

namespace {

using ::fuzztest::domain_implementor::MutationMetadata;

struct WrapperOptions {
    std::string dict_file;
    std::string corpus_dir;
    size_t max_input_size = 10000;
};

WrapperOptions& GetWrapperOptions() {
    static absl::NoDestructor<WrapperOptions> options;
    return *options;
}

// Parse wrapper specific command line flags from @p argc / @p
// argv. These flag/value pairs will be removed from @p argc / @p argv
// and the remaining values compacted.
// @param argc pointer to the argument count (updated in-place on success)
// @param argv pointer to the argument vector (compacted in-place on success)
// @return 0 on Success and 1 on Failure
int ParseWrapperOptions(int* argc, char*** argv) {
    tint::cli::OptionSet opts;

    auto& opt_dict_file = opts.Add<tint::cli::StringOption>(
        "llvm_fuzzer_wrapper_dict_file",
        "Path to dictionary file used by the wrapped legacy LLVMFuzzer target.");
    auto& opt_corpus_dir = opts.Add<tint::cli::StringOption>(
        "llvm_fuzzer_wrapper_corpus_dir",
        "Path to seed corpus directory used by the wrapped legacy LLVMFuzzer target.");
    auto& opt_max_input_size = opts.Add<tint::cli::ValueOption<uint64_t>>(
        "llvm_fuzzer_wrapper_max_input_size",
        "Maximum input size for the wrapped legacy LLVMFuzzer target.",
        tint::cli::Default{uint64_t{10000}});

    tint::cli::ParseOptions parse_opts;
    parse_opts.ignore_unknown = true;
    if (auto res = opts.Parse(argc, argv, parse_opts); res != tint::Success) {
        std::cerr << res.Failure() << "\n";
        return 1;
    }

    auto& wrapper_opts = GetWrapperOptions();
    wrapper_opts.dict_file = opt_dict_file.value.value_or("");
    wrapper_opts.corpus_dir = opt_corpus_dir.value.value_or("");
    wrapper_opts.max_input_size = static_cast<size_t>(opt_max_input_size.value.value_or(10000));
    return 0;
}

/// Reads in seed corpus files and truncates them to the size limit.
/// @return vector of seed byte arrays, or empty if `corpus_dir` is unset
std::vector<std::vector<uint8_t>> ReadByteArraysFromDirectory() {
    const auto& wrapper_opts = GetWrapperOptions();
    if (wrapper_opts.corpus_dir.empty()) {
        return {};
    }
    std::vector<fuzztest::internal::FilePathAndData> files =
        fuzztest::internal::ReadFileOrDirectory(wrapper_opts.corpus_dir);

    std::vector<std::vector<uint8_t>> out;
    out.reserve(files.size());
    for (const fuzztest::internal::FilePathAndData& file : files) {
        const size_t len = std::min(file.data.size(), wrapper_opts.max_input_size);
        out.emplace_back(file.data.begin(), file.data.begin() + static_cast<std::ptrdiff_t>(len));
    }
    return out;
}

/// Reads and parses dictionary file.
/// Aborts if any dictionary file fails to parse.
/// @return vector of dictionary token byte arrays, or empty if `dict_file` is unset
std::vector<std::vector<uint8_t>> ReadByteArrayDictionaryFromFile() {
    const auto& wrapper_opts = GetWrapperOptions();
    if (wrapper_opts.dict_file.empty()) {
        return {};
    }
    std::vector<fuzztest::internal::FilePathAndData> files =
        fuzztest::internal::ReadFileOrDirectory(wrapper_opts.dict_file);

    std::vector<std::vector<uint8_t>> out;
    out.reserve(files.size());
    for (const fuzztest::internal::FilePathAndData& file : files) {
        absl::StatusOr<std::vector<std::string>> parsed_entries =
            fuzztest::ParseDictionary(file.data);
        FUZZTEST_CHECK(parsed_entries.status().ok())
            << "Could not parse dictionary file " << file.path << ": " << parsed_entries.status();
        for (const std::string& parsed_entry : *parsed_entries) {
            out.emplace_back(parsed_entry.begin(), parsed_entry.end());
        }
    }
    return out;
}

/// Bridges FuzzTest's `MutationMetadata` across the C `LLVMFuzzerCustomMutator` boundary so
/// `CentipedeLLVMFuzzerMutateCallback` can access it without changing the C callback signature.
class LLVMFuzzerMutateMetadataManager {
  public:
    /// Takes ownership of @p mutation_metadata for the duration of a custom mutator call.
    /// Must be paired with `Deactivate()` once no readers remain.
    /// @param mutation_metadata metadata for the current mutation step
    void Activate(MutationMetadata mutation_metadata) {
        absl::MutexLock lock(&mu_);
        FUZZTEST_CHECK(!mutation_metadata_.has_value())
            << "MutationMetadata is already active before calling Activate()!";
        FUZZTEST_CHECK(acquire_count_ == 0)
            << "MutationMetadata still has readers before calling Activate()!";
        mutation_metadata_ = std::move(mutation_metadata);
    }

    /// Destroys the active `MutationMetadata`. Requires all `Acquire()` borrows to be released
    /// first.
    void Deactivate() {
        absl::MutexLock lock(&mu_);
        FUZZTEST_CHECK(mutation_metadata_.has_value())
            << "MutationMetadata is not active before calling Deactivate()!";
        FUZZTEST_CHECK(acquire_count_ == 0)
            << "MutationMetadata still has readers before calling Deactivate()!";
        mutation_metadata_ = std::nullopt;
    }

    /// Borrows the active `MutationMetadata`. Must be paired with `Release()` before
    /// `Deactivate()`.
    /// @return borrowed const reference to the active `MutationMetadata`
    const MutationMetadata& Acquire() {
        absl::MutexLock lock(&mu_);
        FUZZTEST_PRECONDITION(mutation_metadata_.has_value())
            << "Cannot acquire unavailable mutation metadata outside LLVMFuzzerCustomMutator().";
        ++acquire_count_;
        return *mutation_metadata_;
    }

    /// Ends a borrow previously started by `Acquire()`.
    void Release() {
        absl::MutexLock lock(&mu_);
        FUZZTEST_CHECK(mutation_metadata_.has_value())
            << "MutationMetadata is not active before calling Release()!";
        FUZZTEST_CHECK(acquire_count_ > 0)
            << "MutationMetadata has no readers before calling Release()!";
        --acquire_count_;
    }

  private:
    absl::Mutex mu_;
    size_t acquire_count_ ABSL_GUARDED_BY(mu_) = 0;
    std::optional<MutationMetadata> mutation_metadata_ ABSL_GUARDED_BY(mu_);
};

absl::NoDestructor<LLVMFuzzerMutateMetadataManager> mutation_metadata_manager;

/// FuzzTest byte-vector domain that integrates `LLVMFuzzerCustomMutator` when linked (e.g., LPM)
/// while falling back to standard byte-vector mutation otherwise.
class ArbitraryByteVector : public fuzztest::internal::SequenceContainerOfImplBase<
                                ArbitraryByteVector,
                                std::vector<uint8_t>,
                                fuzztest::internal::ArbitraryImpl<uint8_t>> {
    using Base = typename ArbitraryByteVector::ContainerOfImplBase;

  public:
    using typename Base::corpus_type;

    ArbitraryByteVector() = default;

    /// Mutates @p val in-place (may resize up to `max_size()`, or keep `<= val.size()` when
    /// @p only_shrink is true).
    /// @param val byte buffer mutated in-place
    /// @param prng random bit generator for mutation
    /// @param metadata FuzzTest mutation metadata
    /// @param only_shrink if true, forbids growing @p val
    void Mutate(corpus_type& val,
                absl::BitGenRef prng,
                const MutationMetadata& metadata,
                bool only_shrink) {
        if (LLVMFuzzerCustomMutator) {
            const size_t size = val.size();
            const size_t mutant_max_size = only_shrink ? size : max_size();
            val.resize(mutant_max_size);
            mutation_metadata_manager->Activate(metadata);
            val.resize(LLVMFuzzerCustomMutator(val.data(), size, mutant_max_size,
                                               static_cast<unsigned int>(prng())));
            mutation_metadata_manager->Deactivate();
        } else {
            Base::Mutate(val, prng, metadata, only_shrink);
        }
    }
};

/// FuzzTest target entrypoint forwarding each input to `LLVMFuzzerTestOneInput`.
/// @param data read-only input bytes for the fuzzer target
void TestOneInput(const std::vector<uint8_t>& data) {
    LLVMFuzzerTestOneInput(data.data(), data.size());
}

FUZZ_TEST(LLVMFuzzer, TestOneInput)
    .WithDomains(fuzztest::internal::Lazy<ArbitraryByteVector>().WithLazySetup([](auto& d) {
        d.WithMaxSize(GetWrapperOptions().max_input_size)
            .WithDictionary(ReadByteArrayDictionaryFromFile)
            .WithSeeds(ReadByteArraysFromDirectory);
    }));

/// Returns whether the process was invoked for direct standalone testcase replay rather than
/// fuzzing or Centipede worker execution.
/// @param argc remaining argument count
/// @param argv remaining argument vector (not mutated)
/// @return true if positional input file replay should be used
bool IsPositionalFileReplay(int argc, char** argv) {
    if (std::getenv("CENTIPEDE_RUNNER_FLAGS") != nullptr || argc <= 1) {
        return false;
    }
    for (int i = 1; i < argc; i++) {
        // SAFETY: `i` is bounded by `[1, argc)` and `argv` has at least `argc` elements.
        std::string_view arg(DAWN_UNSAFE_BUFFERS(argv[i]));
        if (arg.empty() || arg.starts_with('-')) {
            return false;
        }
    }
    return true;
}

/// Replays each positional input file directly through `LLVMFuzzerTestOneInput` for standalone
/// crash reproduction and corpus checking.
/// @param argc argument count
/// @param argv argument vector of input file paths (not mutated)
/// @return 0 on success, 1 if any file cannot be opened
int ReplayFiles(int argc, char** argv) {
    for (int i = 1; i < argc; i++) {
        // SAFETY: `i` is bounded by `[1, argc)` and `argv` has at least `argc` elements.
        const char* path = DAWN_UNSAFE_BUFFERS(argv[i]);
        std::ifstream file(path, std::ios::binary);
        if (!file) {
            std::cerr << "Failed to open input file: " << path << "\n";
            return 1;
        }
        std::vector<uint8_t> data((std::istreambuf_iterator<char>(file)),
                                  std::istreambuf_iterator<char>());
        LLVMFuzzerTestOneInput(data.data(), data.size());
    }
    return 0;
}

}  // namespace

/// C callback backing `LLVMFuzzerMutate` for custom mutators (e.g., LPM) under Centipede.
/// Mutates caller-owned @p data in-place; must only be called while `LLVMFuzzerCustomMutator`
/// has active `MutationMetadata`.
/// @param data caller-owned buffer mutated in-place (capacity at least @p max_size bytes)
/// @param size current byte length of valid input in @p data
/// @param max_size maximum byte length that may be written to @p data
/// @return new byte length written to @p data
extern "C" size_t CentipedeLLVMFuzzerMutateCallback(uint8_t* data, size_t size, size_t max_size) {
    static auto domain = fuzztest::Arbitrary<std::vector<uint8_t>>()
                             .WithDictionary(ReadByteArrayDictionaryFromFile)
                             .WithSeeds(ReadByteArraysFromDirectory);
    domain.WithMaxSize(max_size);
    absl::BitGen bitgen;
    // SAFETY: Caller guarantees `[data, data + size)` is a valid buffer of at least `max_size`
    // bytes.
    std::vector<uint8_t> val(data, DAWN_UNSAFE_BUFFERS(data + size));
    const auto& metadata = mutation_metadata_manager->Acquire();
    domain.Mutate(val, bitgen, metadata, false);
    mutation_metadata_manager->Release();
    std::copy(val.begin(), val.end(), data);
    return val.size();
}

int main(int argc, char** argv) {
    bool saw_help = false;
    for (int i = 1; i < argc; i++) {
        // SAFETY: `i` is bounded by `[1, argc)` and `argv` has at least `argc` elements.
        std::string_view arg(DAWN_UNSAFE_BUFFERS(argv[i]));
        if (arg == "--help" || arg == "-help") {
            saw_help = true;
            break;
        }
    }

    // Parse and strip Tint custom flags before GTest/Abseil/FuzzTest inspect argv.
    if (LLVMFuzzerInitialize) {
        if (int ret = LLVMFuzzerInitialize(&argc, &argv); ret != 0) {
            return ret;
        }
    }

    if (saw_help) {
        return 0;
    }

    if (int ret = ParseWrapperOptions(&argc, &argv); ret != 0) {
        return ret;
    }

    if (IsPositionalFileReplay(argc, argv)) {
        return ReplayFiles(argc, argv);
    }

    testing::InitGoogleTest(&argc, argv);
    fuzztest::ParseAbslFlags(argc, argv);
    fuzztest::InitFuzzTest(&argc, &argv);
    return RUN_ALL_TESTS();
}
