// Copyright 2021 The Dawn Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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
    if (tint::text::utf8::IsASCII(utf8String)) {
        return utf8String.size();
    }

    uint64_t numberOfUTF16CodeUnits = 0;
    std::string_view remaining = utf8String;
    while (!remaining.empty()) {
        auto [codePoint, utf8CharacterByteLength] = tint::text::utf8::Decode(remaining);
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

void OwnedCompilationMessages::AddMessageForTesting(std::string message,
                                                    wgpu::CompilationMessageType type,
                                                    uint64_t lineNum,
                                                    uint64_t linePos,
                                                    uint64_t offset,
                                                    uint64_t length) {
    // Cannot add messages after GetCompilationInfo has been called.
    ASSERT(mCompilationInfo.messages == nullptr);

    // Message can only contain ascii characters.
    ASSERT(tint::text::utf8::IsASCII(message));

    mMessageStrings.push_back(message);
    mMessages.push_back({nullptr, nullptr, static_cast<WGPUCompilationMessageType>(type), lineNum,
                         linePos, offset, length, linePos, offset, length});
}

MaybeError OwnedCompilationMessages::AddMessage(const tint::diag::Diagnostic& diagnostic) {
    // Cannot add messages after GetCompilationInfo has been called.
    ASSERT(mCompilationInfo.messages == nullptr);

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
        ASSERT(endOffsetInBytes >= offsetInBytes);
        lengthInBytes = endOffsetInBytes - offsetInBytes;
        DAWN_TRY_ASSIGN(lengthInUTF16, CountUTF16CodeUnitsFromUTF8String(std::string_view(
                                           fileStart + offsetInBytes, lengthInBytes)));
    }

    if (diagnostic.code) {
        mMessageStrings.push_back(std::string(diagnostic.code) + ": " + diagnostic.message);
    } else {
        mMessageStrings.push_back(diagnostic.message);
    }

    mMessages.push_back({nullptr, nullptr, tintSeverityToMessageType(diagnostic.severity), lineNum,
                         linePosInBytes, offsetInBytes, lengthInBytes, linePosInUTF16,
                         offsetInUTF16, lengthInUTF16});

    return {};
}

MaybeError OwnedCompilationMessages::AddMessages(const tint::diag::List& diagnostics) {
    // Cannot add messages after GetCompilationInfo has been called.
    ASSERT(mCompilationInfo.messages == nullptr);

    for (const auto& diag : diagnostics) {
        DAWN_TRY(AddMessage(diag));
    }

    AddFormattedTintMessages(diagnostics);

    return {};
}

void OwnedCompilationMessages::ClearMessages() {
    // Cannot clear messages after GetCompilationInfo has been called.
    ASSERT(mCompilationInfo.messages == nullptr);

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

const std::vector<std::string>& OwnedCompilationMessages::GetFormattedTintMessages() {
    return mFormattedTintMessages;
}

void OwnedCompilationMessages::AddFormattedTintMessages(const tint::diag::List& diagnostics) {
    tint::diag::List messageList;
    size_t warningCount = 0;
    size_t errorCount = 0;
    for (auto& diag : diagnostics) {
        switch (diag.severity) {
            case (tint::diag::Severity::Fatal):
            case (tint::diag::Severity::Error):
            case (tint::diag::Severity::InternalCompilerError): {
                errorCount++;
                messageList.add(tint::diag::Diagnostic(diag));
                break;
            }
            case (tint::diag::Severity::Warning): {
                warningCount++;
                messageList.add(tint::diag::Diagnostic(diag));
                break;
            }
            case (tint::diag::Severity::Note): {
                messageList.add(tint::diag::Diagnostic(diag));
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
    t << "generated while compiling the shader:" << std::endl
      << tint::diag::Formatter{style}.format(messageList);
    mFormattedTintMessages.push_back(t.str());
}

}  // namespace dawn::native
