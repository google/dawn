#ifndef SRC_TINT_FUZZERS_TINT_STRUCTURE_FUZZER_PROBABILITIES_H_
#define SRC_TINT_FUZZERS_TINT_STRUCTURE_FUZZER_PROBABILITIES_H_

#include <cassert>
#include <vector>
#include "src/tint/fuzzers/random_generator.h"

namespace tint::fuzzers::structure_fuzzer {

struct Probabilities {
    Probabilities(std::vector<unsigned> values_) : values(std::move(values_)) {
        unsigned sum = 0;
        for (unsigned& v : values) {
            unsigned s = v;
            v = sum;
            assert(sum + s > sum);
            sum += s;
        }
        values.push_back(sum);
    }

    size_t size() const { return values.size() - 1; }

    unsigned sum() const { return values.back(); }

    template <typename T>
    T sample(RandomGenerator& gen) const {
        return static_cast<T>(sample(gen));
    }

    unsigned sample(RandomGenerator& gen) const {
        unsigned v = gen.GetUInt32(sum());
        auto it = std::upper_bound(values.begin(), values.end(), v);
        assert(it != values.begin());
        --it;
        return static_cast<unsigned>(std::distance(values.begin(), it));
    }
    std::vector<unsigned> values;
};

}  // namespace tint::fuzzers::structure_fuzzer

#endif
