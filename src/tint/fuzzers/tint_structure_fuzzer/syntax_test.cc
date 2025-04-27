#include <gtest/gtest.h>
#include <vector>
#include <string>
#include <cstdint>
#include "src/tint/fuzzers/tint_structure_fuzzer/syntax.h"
#include "src/tint/fuzzers/random_generator.h"
#include "src/tint/utils/math/crc32.h"

namespace tint::fuzzers::structure_fuzzer {


std::vector<uint8_t> CreateTestData(uint32_t seed, size_t size) {
    RandomGenerator gen(seed);
    std::vector<uint8_t> data;
    for (size_t i = 0; i < size; ++i) {
        data.push_back(gen.GetUInt32(256));
    }
    return data;
}


TEST(SyntaxTest, BasicGeneration) {
    std::vector<uint8_t> data = CreateTestData(42, 1000);
    std::string source = WGSLSource(data.data(), data.size());
    

    EXPECT_FALSE(source.empty());
    

    EXPECT_TRUE(source.find("fn") != std::string::npos || 
                source.find("var") != std::string::npos ||
                source.find("const") != std::string::npos);
}


TEST(SyntaxTest, DifferentSeeds) {
    std::vector<uint8_t> data1 = CreateTestData(42, 1000);
    std::vector<uint8_t> data2 = CreateTestData(43, 1000);
    
    std::string source1 = WGSLSource(data1.data(), data1.size());
    std::string source2 = WGSLSource(data2.data(), data2.size());
    
    EXPECT_NE(source1, source2);
}


TEST(SyntaxTest, MutationChangesOutput) {
    std::vector<uint8_t> data = CreateTestData(42, 1000);
    std::string original = WGSLSource(data.data(), data.size());
    
    RandomGenerator gen(24);
    std::vector<uint8_t> mutated = WGSLMutate(Mutation::RandomTerminal, data.data(), data.size(), gen);
    
    std::string mutated_source = WGSLSource(mutated.data(), mutated.size());
    EXPECT_NE(original, mutated_source);
}


TEST(SyntaxTest, AllMutationTypes) {
    std::vector<uint8_t> data = CreateTestData(42, 5000);
    RandomGenerator gen(24);
    
    for (int i = 0; i <= static_cast<int>(Mutation::Last); ++i) {
        Mutation mutation = static_cast<Mutation>(i);
        std::vector<uint8_t> mutated = WGSLMutate(mutation, data.data(), data.size(), gen);
        

        EXPECT_FALSE(mutated.empty());
        

        std::string source = WGSLSource(mutated.data(), mutated.size());
        EXPECT_FALSE(source.empty());
    }
}


TEST(SyntaxTest, EmptyData) {
    std::vector<uint8_t> empty;
    



    std::string source = WGSLSource(empty.data(), empty.size());
    EXPECT_TRUE(source.empty());
    

    RandomGenerator gen(42);
    std::vector<uint8_t> mutated = WGSLMutate(Mutation::RandomTerminal, empty.data(), empty.size(), gen);
    EXPECT_FALSE(mutated.empty());
}


TEST(SyntaxTest, SmallData) {
    std::vector<uint8_t> small = {42};
    
    std::string source = WGSLSource(small.data(), small.size());
    EXPECT_FALSE(source.empty());
    
    RandomGenerator gen(42);
    std::vector<uint8_t> mutated = WGSLMutate(Mutation::RandomTerminal, small.data(), small.size(), gen);
    EXPECT_FALSE(mutated.empty());
}


TEST(SyntaxTest, SubtreeTransfer) {
    std::vector<uint8_t> data = CreateTestData(42, 5000);
    RandomGenerator gen(24);
    
    std::vector<uint8_t> mutated = WGSLMutate(Mutation::SubtreeTransfer, data.data(), data.size(), gen);
    

    EXPECT_FALSE(mutated.empty());
    

    std::string source = WGSLSource(mutated.data(), mutated.size());
    EXPECT_FALSE(source.empty());
}


TEST(SyntaxTest, Idempotent) {
    std::vector<uint8_t> data = CreateTestData(42, 1000);
    
    RandomGenerator gen1(24);
    std::vector<uint8_t> mutated1 = WGSLMutate(Mutation::RandomTerminal, data.data(), data.size(), gen1);
    
    RandomGenerator gen2(24);
    std::vector<uint8_t> mutated2 = WGSLMutate(Mutation::RandomTerminal, data.data(), data.size(), gen2);
    
    EXPECT_EQ(mutated1, mutated2);
}


TEST(SyntaxTest, AddOptionalMutation) {
    std::vector<uint8_t> data = CreateTestData(42, 1000);
    std::string original = WGSLSource(data.data(), data.size());
    
    RandomGenerator gen(24);
    std::vector<uint8_t> mutated = WGSLMutate(Mutation::AddOptional, data.data(), data.size(), gen);
    

    EXPECT_NE(data, mutated);
    
    std::string mutated_source = WGSLSource(mutated.data(), mutated.size());
    EXPECT_FALSE(mutated_source.empty());
}


TEST(SyntaxTest, RemoveOptionalMutation) {
    std::vector<uint8_t> data = CreateTestData(42, 1000);
    std::string original = WGSLSource(data.data(), data.size());
    
    RandomGenerator gen(24);
    std::vector<uint8_t> mutated = WGSLMutate(Mutation::RemoveOptional, data.data(), data.size(), gen);
    

    EXPECT_NE(data, mutated);
    
    std::string mutated_source = WGSLSource(mutated.data(), mutated.size());
    EXPECT_FALSE(mutated_source.empty());
}


TEST(SyntaxTest, IncRepeatMutation) {
    std::vector<uint8_t> data = CreateTestData(42, 1000);
    std::string original = WGSLSource(data.data(), data.size());
    
    RandomGenerator gen(24);
    std::vector<uint8_t> mutated = WGSLMutate(Mutation::IncRepeat, data.data(), data.size(), gen);
    

    EXPECT_NE(data, mutated);
    
    std::string mutated_source = WGSLSource(mutated.data(), mutated.size());
    EXPECT_FALSE(mutated_source.empty());
}


TEST(SyntaxTest, DecRepeatMutation) {
    std::vector<uint8_t> data = CreateTestData(42, 1000);
    std::string original = WGSLSource(data.data(), data.size());
    
    RandomGenerator gen(24);
    std::vector<uint8_t> mutated = WGSLMutate(Mutation::DecRepeat, data.data(), data.size(), gen);
    

    EXPECT_NE(data, mutated);
    
    std::string mutated_source = WGSLSource(mutated.data(), mutated.size());
    EXPECT_FALSE(mutated_source.empty());
}


TEST(SyntaxTest, NextAlternativeMutation) {
    std::vector<uint8_t> data = CreateTestData(42, 1000);
    std::string original = WGSLSource(data.data(), data.size());
    
    RandomGenerator gen(24);
    std::vector<uint8_t> mutated = WGSLMutate(Mutation::NextAlternative, data.data(), data.size(), gen);
    

    EXPECT_NE(data, mutated);
    
    std::string mutated_source = WGSLSource(mutated.data(), mutated.size());
    EXPECT_FALSE(mutated_source.empty());
}


TEST(SyntaxTest, ChainOfMutations) {
    std::vector<uint8_t> data = CreateTestData(42, 1000);
    std::string original = WGSLSource(data.data(), data.size());
    

    std::vector<Mutation> mutations = {
        Mutation::RandomTerminal,
        Mutation::IncRepeat,
        Mutation::NextAlternative,
        Mutation::SubtreeTransfer,
        Mutation::AddOptional
    };
    
    RandomGenerator gen(24);
    std::vector<uint8_t> current = data;
    
    for (auto mutation : mutations) {
        std::vector<uint8_t> next = WGSLMutate(mutation, current.data(), current.size(), gen);
        

        EXPECT_FALSE(next.empty());
        
        std::string source = WGSLSource(next.data(), next.size());
        EXPECT_FALSE(source.empty());
        
        current = next;
    }
    

    std::string final_source = WGSLSource(current.data(), current.size());
    EXPECT_NE(original, final_source);
}


TEST(SyntaxTest, DifferentSizes) {
    RandomGenerator gen(24);
    
    for (size_t size : {10, 100, 1000}) {
        std::vector<uint8_t> data = CreateTestData(42, size);
        std::string source = WGSLSource(data.data(), data.size());
        

        EXPECT_FALSE(source.empty());
        

        std::vector<uint8_t> mutated = WGSLMutate(Mutation::RandomTerminal, data.data(), data.size(), gen);
        EXPECT_FALSE(mutated.empty());
    }
}


TEST(SyntaxTest, DeterministicMutations) {

    for (int i = 0; i <= static_cast<int>(Mutation::Last); ++i) {
        Mutation mutation = static_cast<Mutation>(i);
        
        std::vector<uint8_t> data = CreateTestData(42, 1000);
        

        RandomGenerator gen1(24);
        std::vector<uint8_t> result1 = WGSLMutate(mutation, data.data(), data.size(), gen1);
        
        RandomGenerator gen2(24);
        std::vector<uint8_t> result2 = WGSLMutate(mutation, data.data(), data.size(), gen2);
        
        EXPECT_EQ(result1, result2) << "Mutation type " << i << " is not deterministic";
        

        RandomGenerator gen3(25);
        std::vector<uint8_t> result3 = WGSLMutate(mutation, data.data(), data.size(), gen3);
        

        if (mutation != Mutation::RandomTerminal && mutation != Mutation::SubtreeTransfer) {
            EXPECT_NE(result1, result3) << "Mutation type " << i << " doesn't depend on random seed";
        }
    }
}

}  // namespace tint::fuzzers::structure_fuzzer

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

