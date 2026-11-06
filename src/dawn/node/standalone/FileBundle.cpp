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

#include "src/dawn/node/standalone/FileBundle.h"

#include <algorithm>
#include <limits>
#include <memory>
#include <span>
#include <utility>

#include "absl/strings/str_split.h"

namespace dawn::node::standalone {
namespace {

// unzReadCurrentFile returns the number of bytes read as an int, so each read must fit in one.
constexpr size_t kMaxReadSize = static_cast<size_t>(std::numeric_limits<int>::max());

// Returns the parent of a bundle path, which is the root ("") for a top-level entry.
std::string_view Parent(std::string_view path) {
    size_t slash = path.rfind('/');
    return slash == std::string_view::npos ? std::string_view() : path.substr(0, slash);
}

// Returns the last component of a bundle path, which is the whole path for a top-level entry.
std::string_view Basename(std::string_view path) {
    size_t slash = path.rfind('/');
    return slash == std::string_view::npos ? path : path.substr(slash + 1);
}

// Strips leading "./" and trailing '/', then returns whether `name` is a non-empty canonical
// relative path under the root so each file and directory has a unique key. Canonical paths do not
// contain ".", "..", or empty components because they would either escape the root or allow the
// same entry to be referenced in multiple ways.
bool NormalizeEntryName(std::string* name) {
    while (name->starts_with("./")) {
        name->erase(0, 2);
    }
    while (!name->empty() && name->back() == '/') {
        name->pop_back();
    }
    // An empty name, a leading '/' and "//" all produce an empty component.
    for (std::string_view component : absl::StrSplit(*name, '/')) {
        if (component.empty() || component == "." || component == "..") {
            return false;
        }
    }
    return true;
}

}  // namespace

dawn::Result<std::unique_ptr<FileBundle>, std::string> FileBundle::OpenZip(
    const std::string& path) {
    unzFile archive = unzOpen64(path.c_str());
    if (archive == nullptr) {
        return std::make_unique<std::string>("not a readable zip archive");
    }
    // The constructor is private, so std::make_unique cannot call it.
    std::unique_ptr<FileBundle> bundle(new FileBundle(archive));
    if (dawn::Result<void, std::string> result = bundle->ReadCentralDirectory(); result.IsError()) {
        return result.AcquireError();
    }
    return std::move(bundle);
}

FileBundle::~FileBundle() {
    unzClose(archive_);
}

void FileBundle::AddDirectory(std::string_view path) {
    if (!directories_.try_emplace(std::string(path)).second) {
        return;
    }
    // Name each new directory in its parent, until a parent that was already registered.
    while (!path.empty()) {
        const std::string_view parent = Parent(path);
        auto [entry, inserted] = directories_.try_emplace(std::string(parent));
        entry->second.emplace_back(Basename(path));
        if (!inserted) {
            break;
        }
        path = parent;
    }
}

dawn::Result<void, std::string> FileBundle::ReadCentralDirectory() {
    // Register the root directory in case the archive is empty.
    AddDirectory("");

    // unz_file_info64 carries a 64 KiB name buffer, so it is not put on the stack.
    auto info = std::make_unique<unz_file_info64>();
    int status = unzGoToFirstFile(archive_);
    for (; status == UNZ_OK; status = unzGoToNextFile(archive_)) {
        unz64_file_pos position;
        status = unzGetCurrentFileInfo64(archive_, info.get(), nullptr, 0, nullptr, 0, nullptr, 0);
        if (status != UNZ_OK) {
            break;
        }
        status = unzGetFilePos64(archive_, &position);
        if (status != UNZ_OK) {
            break;
        }
        std::string name(info->size_filename, '\0');
        const uLong name_size = static_cast<uLong>(name.size());
        status = unzGetCurrentFileInfo64(archive_, info.get(), name.data(), name_size, nullptr, 0,
                                         nullptr, 0);
        if (status != UNZ_OK) {
            break;
        }
        const bool is_directory = name.ends_with('/');
        if (!NormalizeEntryName(&name)) {
            continue;
        }
        if (is_directory) {
            AddDirectory(name);
            continue;
        }
        AddDirectory(Parent(name));
        directories_[std::string(Parent(name))].emplace_back(Basename(name));
        files_[name] = {position, info->uncompressed_size};
    }
    if (status != UNZ_END_OF_LIST_OF_FILE) {
        return std::make_unique<std::string>("the archive's central directory is corrupt");
    }

    for (auto& [directory, names] : directories_) {
        std::ranges::sort(names);
        auto duplicates = std::ranges::unique(names);
        names.erase(duplicates.begin(), duplicates.end());
    }
    return {};
}

FileBundle::Kind FileBundle::Stat(std::string_view path) const {
    if (files_.contains(path)) {
        return Kind::kFile;
    }
    if (directories_.contains(path)) {
        return Kind::kDirectory;
    }
    return Kind::kNotFound;
}

std::optional<std::string> FileBundle::ReadFile(std::string_view path) {
    auto it = files_.find(path);
    if (it == files_.end()) {
        return std::nullopt;
    }
    if (unzGoToFilePos64(archive_, &it->second.position) != UNZ_OK) {
        return std::nullopt;
    }
    if (unzOpenCurrentFile(archive_) != UNZ_OK) {
        return std::nullopt;
    }
    std::string contents(static_cast<size_t>(it->second.size), '\0');
    std::span<char> unfilled_section(contents);
    while (!unfilled_section.empty()) {
        const size_t max_read_size = std::min(unfilled_section.size(), kMaxReadSize);
        const int read_size = unzReadCurrentFile(archive_, unfilled_section.data(),
                                                 static_cast<unsigned>(max_read_size));
        if (read_size <= 0) {
            break;
        }
        unfilled_section = unfilled_section.subspan(static_cast<size_t>(read_size));
    }
    // Closing reports UNZ_CRCERROR when the data read does not match the checksum.
    const bool closed_ok = unzCloseCurrentFile(archive_) == UNZ_OK;
    if (!closed_ok || !unfilled_section.empty()) {
        return std::nullopt;
    }
    return contents;
}

std::optional<std::vector<std::string>> FileBundle::ReadDir(std::string_view path) const {
    auto it = directories_.find(path);
    if (it == directories_.end()) {
        return std::nullopt;
    }
    return it->second;
}

}  // namespace dawn::node::standalone
