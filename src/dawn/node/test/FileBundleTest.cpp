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

#include <gtest/gtest.h>

#include <atomic>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "src/dawn/node/standalone/FileBundle.h"
#include "src/dawn/node/test/TestZip.h"

namespace dawn::node::standalone {
namespace {

using dawn::node::test::WriteZip;
using dawn::node::test::ZipEntry;
using dawn::node::test::ZipMethod;
using Kind = FileBundle::Kind;
using Names = std::vector<std::string>;

class FileBundleTest : public ::testing::Test {
  protected:
    void SetUp() override {
        static std::atomic<uint32_t> counter{0};
        const ::testing::TestInfo* info = ::testing::UnitTest::GetInstance()->current_test_info();
        const std::filesystem::path base = std::filesystem::temp_directory_path();
        while (true) {
            const std::filesystem::path candidate =
                base /
                ("dawn_file_bundle_" + std::string(info->name()) + "_" + std::to_string(counter++));
            std::error_code ec;
            if (std::filesystem::create_directory(candidate, ec)) {
                dir_ = candidate;
                return;
            }
            ASSERT_FALSE(ec) << "failed to create " << candidate << ": " << ec.message();
        }
    }

    void TearDown() override {
        if (!dir_.empty()) {
            std::error_code ec;
            std::filesystem::remove_all(dir_, ec);
        }
    }

    // Writes a zip holding `entries` and opens it as `bundle_`.
    void Open(const std::vector<ZipEntry>& entries, ZipMethod method = ZipMethod::kDeflated) {
        const std::filesystem::path zip = dir_ / "bundle.zip";
        ASSERT_TRUE(WriteZip(zip, entries, method));
        bundle_ = OpenZip(zip);
        ASSERT_NE(bundle_, nullptr);
    }

    // Opens `zip` as a bundle, or records a failure and returns null.
    static std::unique_ptr<FileBundle> OpenZip(const std::filesystem::path& zip) {
        dawn::Result<std::unique_ptr<FileBundle>, std::string> result =
            FileBundle::OpenZip(zip.string());
        if (result.IsError()) {
            ADD_FAILURE() << zip << ": " << *result.AcquireError();
            return nullptr;
        }
        return result.AcquireSuccess();
    }

    // Returns whether FileBundle::OpenZip() rejects `zip`.
    static bool OpenZipFails(const std::filesystem::path& zip) {
        dawn::Result<std::unique_ptr<FileBundle>, std::string> result =
            FileBundle::OpenZip(zip.string());
        if (result.IsError()) {
            result.AcquireError();
            return true;
        }
        result.AcquireSuccess();
        return false;
    }

    std::string Read(const std::string& path) {
        std::optional<std::string> contents = bundle_->ReadFile(path);
        EXPECT_TRUE(contents.has_value()) << path;
        return contents.value_or("");
    }

    std::filesystem::path dir_;
    std::unique_ptr<FileBundle> bundle_;
};

TEST_F(FileBundleTest, OpenZipFailsForAMissingFile) {
    EXPECT_TRUE(OpenZipFails(dir_ / "absent.zip"));
}

TEST_F(FileBundleTest, OpenZipFailsForAFileThatIsNotAZip) {
    const std::filesystem::path text = dir_ / "text.zip";
    std::ofstream(text) << "not a zip";
    EXPECT_TRUE(OpenZipFails(text));
}

TEST_F(FileBundleTest, StatFindsFilesAndTheDirectoriesHoldingThem) {
    Open({{"a/b/c.txt", "c"}});
    EXPECT_EQ(bundle_->Stat("a/b/c.txt"), Kind::kFile);
    EXPECT_EQ(bundle_->Stat("a/b"), Kind::kDirectory);
    EXPECT_EQ(bundle_->Stat("a"), Kind::kDirectory);
    EXPECT_EQ(bundle_->Stat(""), Kind::kDirectory);
    EXPECT_EQ(bundle_->Stat("a/b/c"), Kind::kNotFound);
    EXPECT_EQ(bundle_->Stat("b"), Kind::kNotFound);
}

// minizip decompresses the data. This covers the read loop around it, for both of the methods that
// zip tools write.
TEST_F(FileBundleTest, ReadFileReturnsStoredAndDeflatedContents) {
    // Bytes that are not text, and a run long enough for deflate to shrink.
    const std::string bytes("\x00\x01\xff\r\n", 5);
    const std::string repeated(100000, 'x');
    for (ZipMethod method : {ZipMethod::kStored, ZipMethod::kDeflated}) {
        const std::filesystem::path zip = dir_ / "method.zip";
        ASSERT_TRUE(
            WriteZip(zip, {{"bytes.bin", bytes}, {"big.txt", repeated}, {"empty", ""}}, method));
        std::unique_ptr<FileBundle> bundle = OpenZip(zip);
        ASSERT_NE(bundle, nullptr);
        EXPECT_EQ(bundle->ReadFile("bytes.bin"), bytes);
        EXPECT_EQ(bundle->ReadFile("big.txt"), repeated);
        EXPECT_EQ(bundle->ReadFile("empty"), "");
    }
}

TEST_F(FileBundleTest, ReadFileFailsForDirectoriesAndMissingPaths) {
    Open({{"dir/file.txt", "x"}});
    EXPECT_EQ(bundle_->ReadFile("dir"), std::nullopt);
    EXPECT_EQ(bundle_->ReadFile(""), std::nullopt);
    EXPECT_EQ(bundle_->ReadFile("dir/absent.txt"), std::nullopt);
}

// minizip checks the CRC. This covers that ReadFile reports a mismatch instead of returning the
// damaged data.
TEST_F(FileBundleTest, ReadFileFailsWhenTheDataDoesNotMatchItsChecksum) {
    const std::filesystem::path zip = dir_ / "corrupt.zip";
    ASSERT_TRUE(WriteZip(zip, {{"file.txt", "PAYLOAD"}}, ZipMethod::kStored));
    std::string data;
    {
        std::ifstream in(zip, std::ios::binary);
        data.assign(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
    }
    const size_t payload = data.find("PAYLOAD");
    ASSERT_NE(payload, std::string::npos);
    data[payload] = 'p';
    std::ofstream(zip, std::ios::binary) << data;

    std::unique_ptr<FileBundle> bundle = OpenZip(zip);
    ASSERT_NE(bundle, nullptr);
    EXPECT_EQ(bundle->ReadFile("file.txt"), std::nullopt);
}

TEST_F(FileBundleTest, ReadDirListsEachEntryOnceInOrder) {
    // The first entries create several directories at once.
    Open({{"a/sub/w.js", "w"},
          {"b/c/d/e.txt", "e"},
          {"z.txt", "z"},
          {"a/y.js", "y"},
          {"a/x.js", "x"}});
    EXPECT_EQ(bundle_->ReadDir(""), (Names{"a", "b", "z.txt"}));
    EXPECT_EQ(bundle_->ReadDir("a"), (Names{"sub", "x.js", "y.js"}));
    EXPECT_EQ(bundle_->ReadDir("a/sub"), (Names{"w.js"}));
    EXPECT_EQ(bundle_->ReadDir("b"), (Names{"c"}));
    EXPECT_EQ(bundle_->ReadDir("b/c"), (Names{"d"}));
    EXPECT_EQ(bundle_->ReadDir("b/c/d"), (Names{"e.txt"}));
    EXPECT_EQ(bundle_->ReadDir("z.txt"), std::nullopt);
    EXPECT_EQ(bundle_->ReadDir("q"), std::nullopt);
}

TEST_F(FileBundleTest, DirectoryEntriesAreDirectoriesEvenWhenEmpty) {
    Open({{"empty/", ""}, {"full/", ""}, {"full/f.txt", "f"}});
    EXPECT_EQ(bundle_->Stat("empty"), Kind::kDirectory);
    EXPECT_EQ(bundle_->ReadDir("empty"), Names{});
    EXPECT_EQ(bundle_->ReadDir("full"), Names{"f.txt"});
    EXPECT_EQ(bundle_->ReadDir(""), (Names{"empty", "full"}));
}

TEST_F(FileBundleTest, EntryNamesAreNormalizedAndThoseEscapingTheRootSkipped) {
    Open({{"./dot.txt", "dot"},
          {"../up.txt", "up"},
          {"a/../b.txt", "b"},
          {"/abs.txt", "abs"},
          {"", "a file with no name"}});
    EXPECT_EQ(Read("dot.txt"), "dot");
    EXPECT_EQ(bundle_->ReadDir(""), Names{"dot.txt"});
}

}  // namespace
}  // namespace dawn::node::standalone
