// Copyright 2021 The Dawn & Tint Authors
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

#include "dawn/native/CompilationMessages.h"

#include "dawn/common/Assert.h"
#include "dawn/native/dawn_platform.h"

#include "tint/tint.h"

namespace dawn::native {

namespace {

WGPUCompilationMessageType tintSeverityToMessageType(tint::diag::Severity severity) {
    switch (severity) {
        case tint::diag::Severity::Note:
            return WGPUCompilationMessageType_Info;
        case tint::diag::Severity::Warning:
            return WGPUCompilationMessageType_Warning;
        default:
            return WGPUCompilationMessageType_Error;
    }
}

}  // anonymous namespace

ResultOrError<uint64_t> CountUTF16CodeUnitsFromUTF8String(const std::string_view& utf8String) {
    if (tint::utf8::IsASCII(utf8String)) {
        return utf8String.size();
    }

    uint64_t numberOfUTF16CodeUnits = 0;
    std::string_view remaining = utf8String;
    while (!remaining.empty()) {
        auto [codePoint, utf8CharacterByteLength] = tint::utf8::Decode(remaining);
        // Directly return as something wrong has happened during the UTF-8 decoding.
        if (utf8CharacterByteLength == 0) {
            return DAWN_INTERNAL_ERROR("Fail to decode the unicode string");
        }
        remaining = remaining.substr(utf8CharacterByteLength);

        // Count the number of code units in UTF-16. See https://en.wikipedia.org/wiki/UTF-16 for
        // more details.
        if (codePoint.value <= 0xD7FF || (codePoint.value >= 0xE000 && codePoint.value <= 0xFFFF)) {
            // Code points from U+0000 to U+D7FF and U+E000 to U+FFFF are encoded as single 16-bit
            // code units.
            ++numberOfUTF16CodeUnits;
        } else if (codePoint.value >= 0x10000) {
            // Code points from U+010000 to U+10FFFF are encoded as two 16-bit code units.
            numberOfUTF16CodeUnits += 2;
        } else {
            // UTF-16 cannot encode the code points from U+D800 to U+DFFF.
            return DAWN_INTERNAL_ERROR("The unicode string contains illegal unicode code point.");
        }
    }

    return numberOfUTF16CodeUnits;
}

OwnedCompilationMessages::OwnedCompilationMessages() {
    mCompilationInfo.nextInChain = 0;
    mCompilationInfo.messageCount = 0;
    mCompilationInfo.messages = nullptr;
}

OwnedCompilationMessages::~OwnedCompilationMessages() = default;

void OwnedCompilationMessages::AddUnanchoredMessage(std::string_view message,
                                                    wgpu::CompilationMessageType type) {
    AddMessage(message, {nullptr, nullptr, static_cast<WGPUCompilationMessageType>(type), 0, 0, 0,
                         0, 0, 0, 0});
}

void OwnedCompilationMessages::AddMessageForTesting(std::string_view message,
                                                    wgpu::CompilationMessageType type,
                                                    uint64_t lineNum,
                                                    uint64_t linePos,
                                                    uint64_t offset,
                                                    uint64_t length) {
    AddMessage(message, {nullptr, nullptr, static_cast<WGPUCompilationMessageType>(type), lineNum,
                         linePos, offset, length, linePos, offset, length});
}

MaybeError OwnedCompilationMessages::AddMessage(const tint::diag::Diagnostic& diagnostic) {
    // Tint line and column values are 1-based.
    uint64_t lineNum = diagnostic.source.range.begin.line;
    uint64_t linePosInBytes = diagnostic.source.range.begin.column;
    // The offset is 0-based.
    uint64_t offsetInBytes = 0;
    uint64_t lengthInBytes = 0;
    uint64_t linePosInUTF16 = 0;
    uint64_t offsetInUTF16 = 0;
    uint64_t lengthInUTF16 = 0;

    if (lineNum && linePosInBytes && diagnostic.source.file) {
        const tint::Source::FileContent& content = diagnostic.source.file->content;

        // Tint stores line as std::string_view in a complete source std::string that's in the
        // source file. So to get the offset in bytes of a line we just need to substract its start
        // pointer with the start of the file's content. Note that line numbering in Tint source
        // range starts at 1 while the array of lines start at 0 (hence the -1).
        const char* fileStart = content.data.data();
        const char* lineStart = content.lines[lineNum - 1].data();
        offsetInBytes = static_cast<uint64_t>(lineStart - fileStart) + linePosInBytes - 1;

        // The linePosInBytes is 1-based.
        uint64_t linePosOffsetInUTF16 = 0;
        DAWN_TRY_ASSIGN(linePosOffsetInUTF16, CountUTF16CodeUnitsFromUTF8String(
                                                  std::string_view(lineStart, linePosInBytes - 1)));
        linePosInUTF16 = linePosOffsetInUTF16 + 1;

        // The offset is 0-based.
        uint64_t lineStartToFileStartOffsetInUTF16 = 0;
        DAWN_TRY_ASSIGN(lineStartToFileStartOffsetInUTF16,
                        CountUTF16CodeUnitsFromUTF8String(std::string_view(
                            fileStart, static_cast<uint64_t>(lineStart - fileStart))));
        offsetInUTF16 = lineStartToFileStartOffsetInUTF16 + linePosInUTF16 - 1;

        // If the range has a valid start but the end is not specified, clamp it to the start.
        uint64_t endLineNum = diagnostic.source.range.end.line;
        uint64_t endLineCol = diagnostic.source.range.end.column;
        if (endLineNum == 0 || endLineCol == 0) {
            endLineNum = lineNum;
            endLineCol = linePosInBytes;
        }

        const char* endLineStart = content.lines[endLineNum - 1].data();
        uint64_t endOffsetInBytes =
            static_cast<uint64_t>(endLineStart - fileStart) + endLineCol - 1;
        // The length of the message is the difference between the starting offset and the
        // ending offset. Negative ranges aren't allowed.
        DAWN_ASSERT(endOffsetInBytes >= offsetInBytes);
        lengthInBytes = endOffsetInBytes - offsetInBytes;
        DAWN_TRY_ASSIGN(lengthInUTF16, CountUTF16CodeUnitsFromUTF8String(std::string_view(
                                           fileStart + offsetInBytes, lengthInBytes)));
    }

    AddMessage(
        diagnostic.message.Plain(),
        {nullptr, nullptr, tintSeverityToMessageType(diagnostic.severity), lineNum, linePosInBytes,
         offsetInBytes, lengthInBytes, linePosInUTF16, offsetInUTF16, lengthInUTF16});

    return {};
}

void OwnedCompilationMessages::AddMessage(std::string_view messageString,
                                          const WGPUCompilationMessage& message) {
    // Cannot add messages after GetCompilationInfo has been called.
    DAWN_ASSERT(mCompilationInfo.messages == nullptr);

    DAWN_ASSERT(message.nextInChain == nullptr);
    // The message string won't be populated until GetCompilationInfo.
    DAWN_ASSERT(message.message == nullptr);

    mMessageStrings.push_back(std::string(messageString));
    mMessages.push_back(message);
}

MaybeError OwnedCompilationMessages::AddMessages(const tint::diag::List& diagnostics) {
    // Cannot add messages after GetCompilationInfo has been called.
    DAWN_ASSERT(mCompilationInfo.messages == nullptr);

    for (const auto& diag : diagnostics) {
        DAWN_TRY(AddMessage(diag));
    }

    AddFormattedTintMessages(diagnostics);

    return {};
}

void OwnedCompilationMessages::ClearMessages() {
    // Cannot clear messages after GetCompilationInfo has been called.
    DAWN_ASSERT(mCompilationInfo.messages == nullptr);

    mMessageStrings.clear();
    mMessages.clear();
}

const WGPUCompilationInfo* OwnedCompilationMessages::GetCompilationInfo() {
    mCompilationInfo.messageCount = mMessages.size();
    mCompilationInfo.messages = mMessages.data();

    // Ensure every message points at the correct message string. Cannot do this earlier, since
    // vector reallocations may move the pointers around.
    for (size_t i = 0; i < mCompilationInfo.messageCount; ++i) {
        WGPUCompilationMessage& message = mMessages[i];
        std::string& messageString = mMessageStrings[i];
        message.message = messageString.c_str();
    }

    return &mCompilationInfo;
}

const std::vector<std::string>& OwnedCompilationMessages::GetFormattedTintMessages() const {
    return mFormattedTintMessages;
}

bool OwnedCompilationMessages::HasWarningsOrErrors() const {
    for (const auto& message : mMessages) {
        if (message.type == WGPUCompilationMessageType_Error ||
            message.type == WGPUCompilationMessageType_Warning) {
            return true;
        }
    }
    return false;
}

void OwnedCompilationMessages::AddFormattedTintMessages(const tint::diag::List& diagnostics) {
    tint::diag::List messageList;
    size_t warningCount = 0;
    size_t errorCount = 0;
    for (auto& diag : diagnostics) {
        switch (diag.severity) {
            case tint::diag::Severity::Error: {
                errorCount++;
                messageList.Add(diag);
                break;
            }
            case tint::diag::Severity::Warning: {
                warningCount++;
                messageList.Add(diag);
                break;
            }
            case tint::diag::Severity::Note: {
                messageList.Add(diag);
                break;
            }
            default:
                break;
        }
    }
    if (errorCount == 0 && warningCount == 0) {
        return;
    }
    tint::diag::Formatter::Style style;
    style.print_newline_at_end = false;
    std::ostringstream t;
    if (errorCount > 0) {
        t << errorCount << " error(s) ";
        if (warningCount > 0) {
            t << "and ";
        }
    }
    if (warningCount > 0) {
        t << warningCount << " warning(s) ";
    }
    t << "generated while compiling the shader:\n"
      << tint::diag::Formatter{style}.Format(messageList).Plain();
    mFormattedTintMessages.push_back(t.str());
}

}  // namespace dawn::native
