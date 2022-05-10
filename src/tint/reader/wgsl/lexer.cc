// Copyright 2020 The Tint Authors.
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

#include "src/tint/reader/wgsl/lexer.h"

#include <cctype>
#include <cmath>
#include <cstring>
#include <limits>
#include <optional>  // NOLINT(build/include_order)
#include <tuple>
#include <type_traits>
#include <utility>

#include "src/tint/debug.h"
#include "src/tint/text/unicode.h"

namespace tint::reader::wgsl {
namespace {

// Unicode parsing code assumes that the size of a single std::string element is
// 1 byte.
static_assert(sizeof(decltype(tint::Source::FileContent::data[0])) == sizeof(uint8_t),
              "tint::reader::wgsl requires the size of a std::string element "
              "to be a single byte");

bool read_blankspace(std::string_view str, size_t i, bool* is_blankspace, size_t* blankspace_size) {
    // See https://www.w3.org/TR/WGSL/#blankspace

    auto* utf8 = reinterpret_cast<const uint8_t*>(&str[i]);
    auto [cp, n] = text::utf8::Decode(utf8, str.size() - i);

    if (n == 0) {
        return false;
    }

    static const auto kSpace = text::CodePoint(0x0020);  // space
    static const auto kHTab = text::CodePoint(0x0009);   // horizontal tab
    static const auto kL2R = text::CodePoint(0x200E);    // left-to-right mark
    static const auto kR2L = text::CodePoint(0x200F);    // right-to-left mark

    if (cp == kSpace || cp == kHTab || cp == kL2R || cp == kR2L) {
        *is_blankspace = true;
        *blankspace_size = n;
        return true;
    }

    *is_blankspace = false;
    return true;
}

uint32_t dec_value(char c) {
    if (c >= '0' && c <= '9') {
        return static_cast<uint32_t>(c - '0');
    }
    return 0;
}

uint32_t hex_value(char c) {
    if (c >= '0' && c <= '9') {
        return static_cast<uint32_t>(c - '0');
    }
    if (c >= 'a' && c <= 'f') {
        return 0xA + static_cast<uint32_t>(c - 'a');
    }
    if (c >= 'A' && c <= 'F') {
        return 0xA + static_cast<uint32_t>(c - 'A');
    }
    return 0;
}

/// LimitCheck is the enumerator result of check_limits().
enum class LimitCheck {
    /// The value was within the limits of the data type.
    kWithinLimits,
    /// The value was too small to fit within the data type.
    kTooSmall,
    /// The value was too large to fit within the data type.
    kTooLarge,
};

/// Checks whether the value fits within the integer type `T`
template <typename T>
LimitCheck check_limits(int64_t value) {
    static_assert(std::is_integral_v<T>, "T must be an integer");
    if (value < static_cast<int64_t>(std::numeric_limits<T>::lowest())) {
        return LimitCheck::kTooSmall;
    }
    if (value > static_cast<int64_t>(std::numeric_limits<T>::max())) {
        return LimitCheck::kTooLarge;
    }
    return LimitCheck::kWithinLimits;
}

/// Checks whether the value fits within the floating point type `T`
template <typename T>
LimitCheck check_limits(double value) {
    static_assert(std::is_floating_point_v<T>, "T must be a floating point");
    if (value < static_cast<double>(std::numeric_limits<T>::lowest())) {
        return LimitCheck::kTooSmall;
    }
    if (value > static_cast<double>(std::numeric_limits<T>::max())) {
        return LimitCheck::kTooLarge;
    }
    return LimitCheck::kWithinLimits;
}

}  // namespace

Lexer::Lexer(const Source::File* file) : file_(file), location_{1, 1} {}

Lexer::~Lexer() = default;

const std::string_view Lexer::line() const {
    if (file_->content.lines.size() == 0) {
        static const char* empty_string = "";
        return empty_string;
    }
    return file_->content.lines[location_.line - 1];
}

size_t Lexer::pos() const {
    return location_.column - 1;
}

size_t Lexer::length() const {
    return line().size();
}

const char& Lexer::at(size_t pos) const {
    auto l = line();
    // Unlike for std::string, if pos == l.size(), indexing `l[pos]` is UB for
    // std::string_view.
    if (pos >= l.size()) {
        static const char zero = 0;
        return zero;
    }
    return l[pos];
}

std::string_view Lexer::substr(size_t offset, size_t count) {
    return line().substr(offset, count);
}

void Lexer::advance(size_t offset) {
    location_.column += offset;
}

void Lexer::set_pos(size_t pos) {
    location_.column = pos + 1;
}

void Lexer::advance_line() {
    location_.line++;
    location_.column = 1;
}

bool Lexer::is_eof() const {
    return location_.line >= file_->content.lines.size() && pos() >= length();
}

bool Lexer::is_eol() const {
    return pos() >= length();
}

Token Lexer::next() {
    if (auto t = skip_blankspace_and_comments(); !t.IsUninitialized()) {
        return t;
    }

    if (auto t = try_hex_float(); !t.IsUninitialized()) {
        return t;
    }

    if (auto t = try_hex_integer(); !t.IsUninitialized()) {
        return t;
    }

    if (auto t = try_float(); !t.IsUninitialized()) {
        return t;
    }

    if (auto t = try_integer(); !t.IsUninitialized()) {
        return t;
    }

    if (auto t = try_ident(); !t.IsUninitialized()) {
        return t;
    }

    if (auto t = try_punctuation(); !t.IsUninitialized()) {
        return t;
    }

    return {Token::Type::kError, begin_source(),
            (is_null() ? "null character found" : "invalid character found")};
}

Source Lexer::begin_source() const {
    Source src{};
    src.file = file_;
    src.range.begin = location_;
    src.range.end = location_;
    return src;
}

void Lexer::end_source(Source& src) const {
    src.range.end = location_;
}

bool Lexer::is_null() const {
    return (pos() < length()) && (at(pos()) == 0);
}

bool Lexer::is_digit(char ch) const {
    return std::isdigit(static_cast<unsigned char>(ch));
}

bool Lexer::is_hex(char ch) const {
    return std::isxdigit(static_cast<unsigned char>(ch));
}

bool Lexer::matches(size_t pos, std::string_view sub_string) {
    if (pos >= length())
        return false;
    return substr(pos, sub_string.size()) == sub_string;
}

Token Lexer::skip_blankspace_and_comments() {
    for (;;) {
        auto loc = location_;
        while (!is_eof()) {
            if (is_eol()) {
                advance_line();
                continue;
            }

            bool is_blankspace;
            size_t blankspace_size;
            if (!read_blankspace(line(), pos(), &is_blankspace, &blankspace_size)) {
                return {Token::Type::kError, begin_source(), "invalid UTF-8"};
            }
            if (!is_blankspace) {
                break;
            }

            advance(blankspace_size);
        }

        auto t = skip_comment();
        if (!t.IsUninitialized()) {
            return t;
        }

        // If the cursor didn't advance we didn't remove any blankspace
        // so we're done.
        if (loc == location_)
            break;
    }
    if (is_eof()) {
        return {Token::Type::kEOF, begin_source()};
    }

    return {};
}

Token Lexer::skip_comment() {
    if (matches(pos(), "//")) {
        // Line comment: ignore everything until the end of line.
        while (!is_eol()) {
            if (is_null()) {
                return {Token::Type::kError, begin_source(), "null character found"};
            }
            advance();
        }
        return {};
    }

    if (matches(pos(), "/*")) {
        // Block comment: ignore everything until the closing '*/' token.

        // Record source location of the initial '/*'
        auto source = begin_source();
        source.range.end.column += 1;

        advance(2);

        int depth = 1;
        while (!is_eof() && depth > 0) {
            if (matches(pos(), "/*")) {
                // Start of block comment: increase nesting depth.
                advance(2);
                depth++;
            } else if (matches(pos(), "*/")) {
                // End of block comment: decrease nesting depth.
                advance(2);
                depth--;
            } else if (is_eol()) {
                // Newline: skip and update source location.
                advance_line();
            } else if (is_null()) {
                return {Token::Type::kError, begin_source(), "null character found"};
            } else {
                // Anything else: skip and update source location.
                advance();
            }
        }
        if (depth > 0) {
            return {Token::Type::kError, source, "unterminated block comment"};
        }
    }
    return {};
}

Token Lexer::try_float() {
    auto start = pos();
    auto end = pos();

    auto source = begin_source();
    bool has_mantissa_digits = false;

    if (matches(end, "-")) {
        end++;
    }
    while (end < length() && is_digit(at(end))) {
        has_mantissa_digits = true;
        end++;
    }

    bool has_point = false;
    if (end < length() && matches(end, ".")) {
        has_point = true;
        end++;
    }

    while (end < length() && is_digit(at(end))) {
        has_mantissa_digits = true;
        end++;
    }

    if (!has_mantissa_digits) {
        return {};
    }

    // Parse the exponent if one exists
    bool has_exponent = false;
    if (end < length() && (matches(end, "e") || matches(end, "E"))) {
        end++;
        if (end < length() && (matches(end, "+") || matches(end, "-"))) {
            end++;
        }

        while (end < length() && isdigit(at(end))) {
            has_exponent = true;
            end++;
        }

        // If an 'e' or 'E' was present, then the number part must also be present.
        if (!has_exponent) {
            const auto str = std::string{substr(start, end - start)};
            return {Token::Type::kError, source,
                    "incomplete exponent for floating point literal: " + str};
        }
    }

    bool has_f_suffix = false;
    if (end < length() && matches(end, "f")) {
        end++;
        has_f_suffix = true;
    }

    if (!has_point && !has_exponent && !has_f_suffix) {
        // If it only has digits then it's an integer.
        return {};
    }

    // Save the error string, for use by diagnostics.
    const auto str = std::string{substr(start, end - start)};

    advance(end - start);
    end_source(source);

    double value = strtod(&at(start), nullptr);
    const double magnitude = std::abs(value);

    if (has_f_suffix) {
        // This errors out if a non-zero magnitude is too small to represent in a
        // float. It can't be represented faithfully in an f32.
        if (0.0 < magnitude && magnitude < static_cast<double>(std::numeric_limits<float>::min())) {
            return {Token::Type::kError, source, "magnitude too small to be represented as f32"};
        }
        switch (check_limits<float>(value)) {
            case LimitCheck::kTooSmall:
                return {Token::Type::kError, source, "value too small for f32"};
            case LimitCheck::kTooLarge:
                return {Token::Type::kError, source, "value too large for f32"};
            default:
                return {Token::Type::kFloatFLiteral, source, value};
        }
    }

    // TODO(crbug.com/tint/1504): Properly support abstract float:
    // Change `AbstractFloatType` to `double`, update errors to say 'abstract int'.
    using AbstractFloatType = float;
    if (0.0 < magnitude &&
        magnitude < static_cast<double>(std::numeric_limits<AbstractFloatType>::min())) {
        return {Token::Type::kError, source, "magnitude too small to be represented as f32"};
    }
    switch (check_limits<AbstractFloatType>(value)) {
        case LimitCheck::kTooSmall:
            return {Token::Type::kError, source, "value too small for f32"};
        case LimitCheck::kTooLarge:
            return {Token::Type::kError, source, "value too large for f32"};
        default:
            return {Token::Type::kFloatLiteral, source, value};
    }
}

Token Lexer::try_hex_float() {
    constexpr uint32_t kTotalBits = 32;
    constexpr uint32_t kTotalMsb = kTotalBits - 1;
    constexpr uint32_t kMantissaBits = 23;
    constexpr uint32_t kMantissaMsb = kMantissaBits - 1;
    constexpr uint32_t kMantissaShiftRight = kTotalBits - kMantissaBits;
    constexpr int32_t kExponentBias = 127;
    constexpr int32_t kExponentMax = 255;
    constexpr uint32_t kExponentBits = 8;
    constexpr uint32_t kExponentMask = (1 << kExponentBits) - 1;
    constexpr uint32_t kExponentLeftShift = kMantissaBits;
    constexpr uint32_t kSignBit = 31;

    auto start = pos();
    auto end = pos();

    auto source = begin_source();

    // clang-format off
  // -?0[xX]([0-9a-fA-F]*.?[0-9a-fA-F]+ | [0-9a-fA-F]+.[0-9a-fA-F]*)(p|P)(+|-)?[0-9]+  // NOLINT
    // clang-format on

    // -?
    int32_t sign_bit = 0;
    if (matches(end, "-")) {
        sign_bit = 1;
        end++;
    }
    // 0[xX]
    if (matches(end, "0x") || matches(end, "0X")) {
        end += 2;
    } else {
        return {};
    }

    uint32_t mantissa = 0;
    uint32_t exponent = 0;

    // TODO(dneto): Values in the normal range for the format do not explicitly
    // store the most significant bit.  The algorithm here works hard to eliminate
    // that bit in the representation during parsing, and then it backtracks
    // when it sees it may have to explicitly represent it, and backtracks again
    // when it sees the number is sub-normal (i.e. the exponent underflows).
    // I suspect the logic can be clarified by storing it during parsing, and
    // then removing it later only when needed.

    // `set_next_mantissa_bit_to` sets next `mantissa` bit starting from msb to
    // lsb to value 1 if `set` is true, 0 otherwise. Returns true on success, i.e.
    // when the bit can be accommodated in the available space.
    uint32_t mantissa_next_bit = kTotalMsb;
    auto set_next_mantissa_bit_to = [&](bool set, bool integer_part) -> bool {
        // If adding bits for the integer part, we can overflow whether we set the
        // bit or not. For the fractional part, we can only overflow when setting
        // the bit.
        const bool check_overflow = integer_part || set;
        // Note: mantissa_next_bit actually decrements, so comparing it as
        // larger than a positive number relies on wraparound.
        if (check_overflow && (mantissa_next_bit > kTotalMsb)) {
            return false;  // Overflowed mantissa
        }
        if (set) {
            mantissa |= (1 << mantissa_next_bit);
        }
        --mantissa_next_bit;
        return true;
    };

    // Collect integer range (if any)
    auto integer_range = std::make_pair(end, end);
    while (end < length() && is_hex(at(end))) {
        integer_range.second = ++end;
    }

    // .?
    bool hex_point = false;
    if (matches(end, ".")) {
        hex_point = true;
        end++;
    }

    // Collect fractional range (if any)
    auto fractional_range = std::make_pair(end, end);
    while (end < length() && is_hex(at(end))) {
        fractional_range.second = ++end;
    }

    // Must have at least an integer or fractional part
    if ((integer_range.first == integer_range.second) &&
        (fractional_range.first == fractional_range.second)) {
        return {};
    }

    // Is the binary exponent present?  It's optional.
    const bool has_exponent = (matches(end, "p") || matches(end, "P"));
    if (has_exponent) {
        end++;
    }
    if (!has_exponent && !hex_point) {
        // It's not a hex float. At best it's a hex integer.
        return {};
    }

    // At this point, we know for sure our token is a hex float value,
    // or an invalid token.

    // Parse integer part
    // [0-9a-fA-F]*

    bool has_zero_integer = true;
    // The magnitude is zero if and only if seen_prior_one_bits is false.
    bool seen_prior_one_bits = false;
    for (auto i = integer_range.first; i < integer_range.second; ++i) {
        const auto nibble = hex_value(at(i));
        if (nibble != 0) {
            has_zero_integer = false;
        }

        for (int32_t bit = 3; bit >= 0; --bit) {
            auto v = 1 & (nibble >> bit);

            // Skip leading 0s and the first 1
            if (seen_prior_one_bits) {
                if (!set_next_mantissa_bit_to(v != 0, true)) {
                    return {Token::Type::kError, source, "mantissa is too large for hex float"};
                }
                ++exponent;
            } else {
                if (v == 1) {
                    seen_prior_one_bits = true;
                }
            }
        }
    }

    // Parse fractional part
    // [0-9a-fA-F]*
    for (auto i = fractional_range.first; i < fractional_range.second; ++i) {
        auto nibble = hex_value(at(i));
        for (int32_t bit = 3; bit >= 0; --bit) {
            auto v = 1 & (nibble >> bit);

            if (v == 1) {
                seen_prior_one_bits = true;
            }

            // If integer part is 0, we only start writing bits to the
            // mantissa once we have a non-zero fractional bit. While the fractional
            // values are 0, we adjust the exponent to avoid overflowing `mantissa`.
            if (!seen_prior_one_bits) {
                --exponent;
            } else {
                if (!set_next_mantissa_bit_to(v != 0, false)) {
                    return {Token::Type::kError, source, "mantissa is too large for hex float"};
                }
            }
        }
    }

    // Determine if the value of the mantissa is zero.
    // Note: it's not enough to check mantissa == 0 as we drop the initial bit,
    // whether it's in the integer part or the fractional part.
    const bool is_zero = !seen_prior_one_bits;
    TINT_ASSERT(Reader, !is_zero || mantissa == 0);

    // Parse the optional exponent.
    // ((p|P)(\+|-)?[0-9]+)?
    uint32_t input_exponent = 0;  // Defaults to 0 if not present
    int32_t exponent_sign = 1;
    // If the 'p' part is present, the rest of the exponent must exist.
    bool has_f_suffix = false;
    if (has_exponent) {
        // Parse the rest of the exponent.
        // (+|-)?
        if (matches(end, "+")) {
            end++;
        } else if (matches(end, "-")) {
            exponent_sign = -1;
            end++;
        }

        // Parse exponent from input
        // [0-9]+
        // Allow overflow (in uint32_t) when the floating point value magnitude is
        // zero.
        bool has_exponent_digits = false;
        while (end < length() && isdigit(at(end))) {
            has_exponent_digits = true;
            auto prev_exponent = input_exponent;
            input_exponent = (input_exponent * 10) + dec_value(at(end));
            // Check if we've overflowed input_exponent. This only matters when
            // the mantissa is non-zero.
            if (!is_zero && (prev_exponent > input_exponent)) {
                return {Token::Type::kError, source, "exponent is too large for hex float"};
            }
            end++;
        }

        // Parse optional 'f' suffix.  For a hex float, it can only exist
        // when the exponent is present. Otherwise it will look like
        // one of the mantissa digits.
        if (end < length() && matches(end, "f")) {
            has_f_suffix = true;
            end++;
        }

        if (!has_exponent_digits) {
            return {Token::Type::kError, source, "expected an exponent value for hex float"};
        }
    }

    advance(end - start);
    end_source(source);

    if (is_zero) {
        // If value is zero, then ignore the exponent and produce a zero
        exponent = 0;
    } else {
        // Ensure input exponent is not too large; i.e. that it won't overflow when
        // adding the exponent bias.
        const uint32_t kIntMax = static_cast<uint32_t>(std::numeric_limits<int32_t>::max());
        const uint32_t kMaxInputExponent = kIntMax - kExponentBias;
        if (input_exponent > kMaxInputExponent) {
            return {Token::Type::kError, source, "exponent is too large for hex float"};
        }

        // Compute exponent so far
        exponent += static_cast<uint32_t>(static_cast<int32_t>(input_exponent) * exponent_sign);

        // Bias exponent if non-zero
        // After this, if exponent is <= 0, our value is a denormal
        exponent += kExponentBias;

        // We know the number is not zero.  The MSB is 1 (by construction), and
        // should be eliminated because it becomes the implicit 1 that isn't
        // explicitly represented in the binary32 format.  We'll bring it back
        // later if we find the exponent actually underflowed, i.e. the number
        // is sub-normal.
        if (has_zero_integer) {
            mantissa <<= 1;
            --exponent;
        }
    }

    // We can now safely work with exponent as a signed quantity, as there's no
    // chance to overflow
    int32_t signed_exponent = static_cast<int32_t>(exponent);

    // Shift mantissa to occupy the low 23 bits
    mantissa >>= kMantissaShiftRight;

    // If denormal, shift mantissa until our exponent is zero
    if (!is_zero) {
        // Denorm has exponent 0 and non-zero mantissa. We set the top bit here,
        // then shift the mantissa to make exponent zero.
        if (signed_exponent <= 0) {
            mantissa >>= 1;
            mantissa |= (1 << kMantissaMsb);
        }

        while (signed_exponent < 0) {
            mantissa >>= 1;
            ++signed_exponent;

            // If underflow, clamp to zero
            if (mantissa == 0) {
                signed_exponent = 0;
            }
        }
    }

    if (signed_exponent > kExponentMax) {
        // Overflow: set to infinity
        signed_exponent = kExponentMax;
        mantissa = 0;
    } else if (signed_exponent == kExponentMax && mantissa != 0) {
        // NaN: set to infinity
        mantissa = 0;
    }

    // Combine sign, mantissa, and exponent
    uint32_t result_u32 = sign_bit << kSignBit;
    result_u32 |= mantissa;
    result_u32 |= (static_cast<uint32_t>(signed_exponent) & kExponentMask) << kExponentLeftShift;

    // Reinterpret as float and return
    float result_f32;
    std::memcpy(&result_f32, &result_u32, sizeof(result_f32));
    double result_f64 = static_cast<double>(result_f32);
    return {has_f_suffix ? Token::Type::kFloatFLiteral : Token::Type::kFloatLiteral, source,
            result_f64};
}

Token Lexer::build_token_from_int_if_possible(Source source, size_t start, int32_t base) {
    int64_t res = strtoll(&at(start), nullptr, base);

    if (matches(pos(), "u")) {
        switch (check_limits<uint32_t>(res)) {
            case LimitCheck::kTooSmall:
                return {Token::Type::kError, source, "unsigned literal cannot be negative"};
            case LimitCheck::kTooLarge:
                return {Token::Type::kError, source, "value too large for u32"};
            default:
                advance(1);
                end_source(source);
                return {Token::Type::kIntULiteral, source, res};
        }
    }

    if (matches(pos(), "i")) {
        switch (check_limits<int32_t>(res)) {
            case LimitCheck::kTooSmall:
                return {Token::Type::kError, source, "value too small for i32"};
            case LimitCheck::kTooLarge:
                return {Token::Type::kError, source, "value too large for i32"};
            default:
                break;
        }
        advance(1);
        end_source(source);
        return {Token::Type::kIntILiteral, source, res};
    }

    // TODO(crbug.com/tint/1504): Properly support abstract int:
    // Change `AbstractIntType` to `int64_t`, update errors to say 'abstract int'.
    using AbstractIntType = int32_t;
    switch (check_limits<AbstractIntType>(res)) {
        case LimitCheck::kTooSmall:
            return {Token::Type::kError, source, "value too small for i32"};
        case LimitCheck::kTooLarge:
            return {Token::Type::kError, source, "value too large for i32"};
        default:
            end_source(source);
            return {Token::Type::kIntLiteral, source, res};
    }
}

Token Lexer::try_hex_integer() {
    constexpr size_t kMaxDigits = 8;  // Valid for both 32-bit integer types
    auto start = pos();
    auto end = pos();

    auto source = begin_source();

    if (matches(end, "-")) {
        end++;
    }

    if (matches(end, "0x") || matches(end, "0X")) {
        end += 2;
    } else {
        return {};
    }

    auto first = end;
    while (!is_eol() && is_hex(at(end))) {
        end++;

        auto digits = end - first;
        if (digits > kMaxDigits) {
            return {Token::Type::kError, source,
                    "integer literal (" + std::string{substr(start, end - 1 - start)} +
                        "...) has too many digits"};
        }
    }
    if (first == end) {
        return {Token::Type::kError, source,
                "integer or float hex literal has no significant digits"};
    }

    advance(end - start);

    return build_token_from_int_if_possible(source, start, 16);
}

Token Lexer::try_integer() {
    constexpr size_t kMaxDigits = 10;  // Valid for both 32-bit integer types
    auto start = pos();
    auto end = start;

    auto source = begin_source();

    if (matches(end, "-")) {
        end++;
    }

    if (end >= length() || !is_digit(at(end))) {
        return {};
    }

    auto first = end;
    // If the first digit is a zero this must only be zero as leading zeros
    // are not allowed.
    auto next = first + 1;
    if (next < length()) {
        if (at(first) == '0' && is_digit(at(next))) {
            return {Token::Type::kError, source,
                    "integer literal (" + std::string{substr(start, end - 1 - start)} +
                        "...) has leading 0s"};
        }
    }

    while (end < length() && is_digit(at(end))) {
        auto digits = end - first;
        if (digits > kMaxDigits) {
            return {Token::Type::kError, source,
                    "integer literal (" + std::string{substr(start, end - 1 - start)} +
                        "...) has too many digits"};
        }

        end++;
    }

    advance(end - start);

    return build_token_from_int_if_possible(source, start, 10);
}

Token Lexer::try_ident() {
    auto source = begin_source();
    auto start = pos();

    // Must begin with an XID_Source unicode character, or underscore
    {
        auto* utf8 = reinterpret_cast<const uint8_t*>(&at(pos()));
        auto [code_point, n] = text::utf8::Decode(utf8, length() - pos());
        if (n == 0) {
            advance();  // Skip the bad byte.
            return {Token::Type::kError, source, "invalid UTF-8"};
        }
        if (code_point != text::CodePoint('_') && !code_point.IsXIDStart()) {
            return {};
        }
        // Consume start codepoint
        advance(n);
    }

    while (!is_eol()) {
        // Must continue with an XID_Continue unicode character
        auto* utf8 = reinterpret_cast<const uint8_t*>(&at(pos()));
        auto [code_point, n] = text::utf8::Decode(utf8, line().size() - pos());
        if (n == 0) {
            advance();  // Skip the bad byte.
            return {Token::Type::kError, source, "invalid UTF-8"};
        }
        if (!code_point.IsXIDContinue()) {
            break;
        }

        // Consume continuing codepoint
        advance(n);
    }

    if (at(start) == '_') {
        // Check for an underscore on its own (special token), or a
        // double-underscore (not allowed).
        if ((pos() == start + 1) || (at(start + 1) == '_')) {
            set_pos(start);
            return {};
        }
    }

    auto str = substr(start, pos() - start);
    end_source(source);

    auto t = check_keyword(source, str);
    if (!t.IsUninitialized()) {
        return t;
    }

    return {Token::Type::kIdentifier, source, str};
}

Token Lexer::try_punctuation() {
    auto source = begin_source();
    auto type = Token::Type::kUninitialized;

    if (matches(pos(), "@")) {
        type = Token::Type::kAttr;
        advance(1);
    } else if (matches(pos(), "(")) {
        type = Token::Type::kParenLeft;
        advance(1);
    } else if (matches(pos(), ")")) {
        type = Token::Type::kParenRight;
        advance(1);
    } else if (matches(pos(), "[")) {
        type = Token::Type::kBracketLeft;
        advance(1);
    } else if (matches(pos(), "]")) {
        type = Token::Type::kBracketRight;
        advance(1);
    } else if (matches(pos(), "{")) {
        type = Token::Type::kBraceLeft;
        advance(1);
    } else if (matches(pos(), "}")) {
        type = Token::Type::kBraceRight;
        advance(1);
    } else if (matches(pos(), "&&")) {
        type = Token::Type::kAndAnd;
        advance(2);
    } else if (matches(pos(), "&=")) {
        type = Token::Type::kAndEqual;
        advance(2);
    } else if (matches(pos(), "&")) {
        type = Token::Type::kAnd;
        advance(1);
    } else if (matches(pos(), "/=")) {
        type = Token::Type::kDivisionEqual;
        advance(2);
    } else if (matches(pos(), "/")) {
        type = Token::Type::kForwardSlash;
        advance(1);
    } else if (matches(pos(), "!=")) {
        type = Token::Type::kNotEqual;
        advance(2);
    } else if (matches(pos(), "!")) {
        type = Token::Type::kBang;
        advance(1);
    } else if (matches(pos(), ":")) {
        type = Token::Type::kColon;
        advance(1);
    } else if (matches(pos(), ",")) {
        type = Token::Type::kComma;
        advance(1);
    } else if (matches(pos(), "==")) {
        type = Token::Type::kEqualEqual;
        advance(2);
    } else if (matches(pos(), "=")) {
        type = Token::Type::kEqual;
        advance(1);
    } else if (matches(pos(), ">=")) {
        type = Token::Type::kGreaterThanEqual;
        advance(2);
    } else if (matches(pos(), ">>")) {
        type = Token::Type::kShiftRight;
        advance(2);
    } else if (matches(pos(), ">")) {
        type = Token::Type::kGreaterThan;
        advance(1);
    } else if (matches(pos(), "<=")) {
        type = Token::Type::kLessThanEqual;
        advance(2);
    } else if (matches(pos(), "<<")) {
        type = Token::Type::kShiftLeft;
        advance(2);
    } else if (matches(pos(), "<")) {
        type = Token::Type::kLessThan;
        advance(1);
    } else if (matches(pos(), "%=")) {
        type = Token::Type::kModuloEqual;
        advance(2);
    } else if (matches(pos(), "%")) {
        type = Token::Type::kMod;
        advance(1);
    } else if (matches(pos(), "->")) {
        type = Token::Type::kArrow;
        advance(2);
    } else if (matches(pos(), "--")) {
        type = Token::Type::kMinusMinus;
        advance(2);
    } else if (matches(pos(), "-=")) {
        type = Token::Type::kMinusEqual;
        advance(2);
    } else if (matches(pos(), "-")) {
        type = Token::Type::kMinus;
        advance(1);
    } else if (matches(pos(), ".")) {
        type = Token::Type::kPeriod;
        advance(1);
    } else if (matches(pos(), "++")) {
        type = Token::Type::kPlusPlus;
        advance(2);
    } else if (matches(pos(), "+=")) {
        type = Token::Type::kPlusEqual;
        advance(2);
    } else if (matches(pos(), "+")) {
        type = Token::Type::kPlus;
        advance(1);
    } else if (matches(pos(), "||")) {
        type = Token::Type::kOrOr;
        advance(2);
    } else if (matches(pos(), "|=")) {
        type = Token::Type::kOrEqual;
        advance(2);
    } else if (matches(pos(), "|")) {
        type = Token::Type::kOr;
        advance(1);
    } else if (matches(pos(), ";")) {
        type = Token::Type::kSemicolon;
        advance(1);
    } else if (matches(pos(), "*=")) {
        type = Token::Type::kTimesEqual;
        advance(2);
    } else if (matches(pos(), "*")) {
        type = Token::Type::kStar;
        advance(1);
    } else if (matches(pos(), "~")) {
        type = Token::Type::kTilde;
        advance(1);
    } else if (matches(pos(), "_")) {
        type = Token::Type::kUnderscore;
        advance(1);
    } else if (matches(pos(), "^=")) {
        type = Token::Type::kXorEqual;
        advance(2);
    } else if (matches(pos(), "^")) {
        type = Token::Type::kXor;
        advance(1);
    }

    end_source(source);

    return {type, source};
}

Token Lexer::check_keyword(const Source& source, std::string_view str) {
    if (str == "array")
        return {Token::Type::kArray, source, "array"};
    if (str == "atomic")
        return {Token::Type::kAtomic, source, "atomic"};
    if (str == "bitcast")
        return {Token::Type::kBitcast, source, "bitcast"};
    if (str == "bool")
        return {Token::Type::kBool, source, "bool"};
    if (str == "break")
        return {Token::Type::kBreak, source, "break"};
    if (str == "case")
        return {Token::Type::kCase, source, "case"};
    if (str == "continue")
        return {Token::Type::kContinue, source, "continue"};
    if (str == "continuing")
        return {Token::Type::kContinuing, source, "continuing"};
    if (str == "discard")
        return {Token::Type::kDiscard, source, "discard"};
    if (str == "default")
        return {Token::Type::kDefault, source, "default"};
    if (str == "else")
        return {Token::Type::kElse, source, "else"};
    if (str == "enable")
        return {Token::Type::kEnable, source, "enable"};
    if (str == "f32")
        return {Token::Type::kF32, source, "f32"};
    if (str == "fallthrough")
        return {Token::Type::kFallthrough, source, "fallthrough"};
    if (str == "false")
        return {Token::Type::kFalse, source, "false"};
    if (str == "fn")
        return {Token::Type::kFn, source, "fn"};
    if (str == "for")
        return {Token::Type::kFor, source, "for"};
    if (str == "function")
        return {Token::Type::kFunction, source, "function"};
    if (str == "i32")
        return {Token::Type::kI32, source, "i32"};
    if (str == "if")
        return {Token::Type::kIf, source, "if"};
    if (str == "import")
        return {Token::Type::kImport, source, "import"};
    if (str == "let")
        return {Token::Type::kLet, source, "let"};
    if (str == "loop")
        return {Token::Type::kLoop, source, "loop"};
    if (str == "mat2x2")
        return {Token::Type::kMat2x2, source, "mat2x2"};
    if (str == "mat2x3")
        return {Token::Type::kMat2x3, source, "mat2x3"};
    if (str == "mat2x4")
        return {Token::Type::kMat2x4, source, "mat2x4"};
    if (str == "mat3x2")
        return {Token::Type::kMat3x2, source, "mat3x2"};
    if (str == "mat3x3")
        return {Token::Type::kMat3x3, source, "mat3x3"};
    if (str == "mat3x4")
        return {Token::Type::kMat3x4, source, "mat3x4"};
    if (str == "mat4x2")
        return {Token::Type::kMat4x2, source, "mat4x2"};
    if (str == "mat4x3")
        return {Token::Type::kMat4x3, source, "mat4x3"};
    if (str == "mat4x4")
        return {Token::Type::kMat4x4, source, "mat4x4"};
    if (str == "override")
        return {Token::Type::kOverride, source, "override"};
    if (str == "private")
        return {Token::Type::kPrivate, source, "private"};
    if (str == "ptr")
        return {Token::Type::kPtr, source, "ptr"};
    if (str == "return")
        return {Token::Type::kReturn, source, "return"};
    if (str == "sampler")
        return {Token::Type::kSampler, source, "sampler"};
    if (str == "sampler_comparison")
        return {Token::Type::kComparisonSampler, source, "sampler_comparison"};
    if (str == "storage_buffer" || str == "storage")
        return {Token::Type::kStorage, source, "storage"};
    if (str == "struct")
        return {Token::Type::kStruct, source, "struct"};
    if (str == "switch")
        return {Token::Type::kSwitch, source, "switch"};
    if (str == "texture_1d")
        return {Token::Type::kTextureSampled1d, source, "texture_1d"};
    if (str == "texture_2d")
        return {Token::Type::kTextureSampled2d, source, "texture_2d"};
    if (str == "texture_2d_array")
        return {Token::Type::kTextureSampled2dArray, source, "texture_2d_array"};
    if (str == "texture_3d")
        return {Token::Type::kTextureSampled3d, source, "texture_3d"};
    if (str == "texture_cube")
        return {Token::Type::kTextureSampledCube, source, "texture_cube"};
    if (str == "texture_cube_array") {
        return {Token::Type::kTextureSampledCubeArray, source, "texture_cube_array"};
    }
    if (str == "texture_depth_2d")
        return {Token::Type::kTextureDepth2d, source, "texture_depth_2d"};
    if (str == "texture_depth_2d_array") {
        return {Token::Type::kTextureDepth2dArray, source, "texture_depth_2d_array"};
    }
    if (str == "texture_depth_cube")
        return {Token::Type::kTextureDepthCube, source, "texture_depth_cube"};
    if (str == "texture_depth_cube_array") {
        return {Token::Type::kTextureDepthCubeArray, source, "texture_depth_cube_array"};
    }
    if (str == "texture_depth_multisampled_2d") {
        return {Token::Type::kTextureDepthMultisampled2d, source, "texture_depth_multisampled_2d"};
    }
    if (str == "texture_external") {
        return {Token::Type::kTextureExternal, source, "texture_external"};
    }
    if (str == "texture_multisampled_2d") {
        return {Token::Type::kTextureMultisampled2d, source, "texture_multisampled_2d"};
    }
    if (str == "texture_storage_1d") {
        return {Token::Type::kTextureStorage1d, source, "texture_storage_1d"};
    }
    if (str == "texture_storage_2d") {
        return {Token::Type::kTextureStorage2d, source, "texture_storage_2d"};
    }
    if (str == "texture_storage_2d_array") {
        return {Token::Type::kTextureStorage2dArray, source, "texture_storage_2d_array"};
    }
    if (str == "texture_storage_3d") {
        return {Token::Type::kTextureStorage3d, source, "texture_storage_3d"};
    }
    if (str == "true")
        return {Token::Type::kTrue, source, "true"};
    if (str == "type")
        return {Token::Type::kType, source, "type"};
    if (str == "u32")
        return {Token::Type::kU32, source, "u32"};
    if (str == "uniform")
        return {Token::Type::kUniform, source, "uniform"};
    if (str == "var")
        return {Token::Type::kVar, source, "var"};
    if (str == "vec2")
        return {Token::Type::kVec2, source, "vec2"};
    if (str == "vec3")
        return {Token::Type::kVec3, source, "vec3"};
    if (str == "vec4")
        return {Token::Type::kVec4, source, "vec4"};
    if (str == "workgroup")
        return {Token::Type::kWorkgroup, source, "workgroup"};
    return {};
}

}  // namespace tint::reader::wgsl
