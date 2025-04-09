
#ifndef SRC_TINT_FUZZERS_TINT_STRUCTURE_FUZZER_SYNTAX_H_
#define SRC_TINT_FUZZERS_TINT_STRUCTURE_FUZZER_SYNTAX_H_

#include <string>
#include <vector>
#include "src/tint/fuzzers/random_generator.h"

namespace tint::fuzzers::structure_fuzzer {

enum class Mutation {
    AddOptional,        // ?
    RemoveOptional,     // ?
    IncRepeat,          // *
    DecRepeat,          // *
    NextAlternative,    // |
    PrevAlternative,    // |
    RandomAlternative,  // |
    RandomTerminal,     //

    Last = RandomTerminal,
};

void WGSLInit();

std::vector<uint8_t> WGSLMutate(Mutation mutation,
                                const uint8_t* data,
                                size_t size,
                                RandomGenerator& gen);

std::string WGSLSource(const uint8_t* data, size_t size);

}  // namespace tint::fuzzers::structure_fuzzer

#endif
