{{- (Globals).Put "enum_override_names" Map -}}

{{- /* ------------------------------------------------------------------ */ -}}
{{-                      define "OverrideEnumName"                           -}}
{{- /* Overrides the C++ name for a sem.Enum.                             */ -}}
{{- /* Arguments:                                                         */ -}}
{{- /* * 'Enum' the sem::Enum                                             */ -}}
{{- /* * 'Name' the new C++ name for enum                                 */ -}}
{{- /* ------------------------------------------------------------------ */ -}}
{{-   $enum_override_names := (Globals).Get "enum_override_names" -}}
{{-   $enum_override_names.Put $.Enum $.Name -}}
{{- end -}}


{{- /* ------------------------------------------------------------------ */ -}}
{{-                         define "EnumName"                                -}}
{{- /* Prints the C++ name for the given sem.Enum argument.               */ -}}
{{- /* ------------------------------------------------------------------ */ -}}
{{-   $enum_override_names := (Globals).Get "enum_override_names" -}}
{{-   $override := $enum_override_names.Get $ -}}
{{    if $override -}}
{{        $override  -}}
{{    else -}}
{{        PascalCase $.Name}}
{{-   end -}}
{{- end -}}


{{- /* ------------------------------------------------------------------ */ -}}
{{-                         define "EnumCase"                                -}}
{{- /* Prints the 'Enum::kEntry' name for the provided sem.EnumEntry      */ -}}
{{- /* argument.                                                          */ -}}
{{- /* ------------------------------------------------------------------ */ -}}
{{-   Eval "EnumName" $.Enum}}::k{{PascalCase $.Name}}
{{- end -}}


{{- /* ------------------------------------------------------------------ */ -}}
{{-                         define "DeclareEnum"                             -}}
{{- /* Declares the 'enum class' for the provided sem.Enum argument.      */ -}}
{{- /* ------------------------------------------------------------------ */ -}}
{{- $enum := Eval "EnumName" $ -}}
enum class {{$enum}} {
    kUndefined,
{{-   range $entry := $.Entries }}
    k{{PascalCase $entry.Name}},{{if $entry.IsInternal}}  // Tint-internal enum entry - not parsed{{end}}
{{-   end }}
};

/// @param out the stream to write to
/// @param value the {{$enum}}
/// @returns `out` so calls can be chained
utils::StringStream& operator<<(utils::StringStream& out, {{$enum}} value);

/// Parse{{$enum}} parses a {{$enum}} from a string.
/// @param str the string to parse
/// @returns the parsed enum, or {{$enum}}::kUndefined if the string could not be parsed.
{{$enum}} Parse{{$enum}}(std::string_view str);

constexpr const char* k{{$enum}}Strings[] = {
{{-   range $entry := $.Entries }}
{{-     if not $entry.IsInternal}}
    "{{$entry.Name}}",
{{-     end }}
{{-   end }}
};

{{- end -}}


{{- /* ------------------------------------------------------------------ */ -}}
{{-                          define "ParseEnum"                              -}}
{{- /* Implements the 'ParseEnum' function for the provided sem.Enum      */ -}}
{{- /* argument.                                                          */ -}}
{{- /* ------------------------------------------------------------------ */ -}}
{{- $enum := Eval "EnumName" $ -}}
/// Parse{{$enum}} parses a {{$enum}} from a string.
/// @param str the string to parse
/// @returns the parsed enum, or {{$enum}}::kUndefined if the string could not be parsed.
{{$enum}} Parse{{$enum}}(std::string_view str) {
{{-   range $entry := $.PublicEntries }}
    if (str == "{{$entry.Name}}") {
        return {{template "EnumCase" $entry}};
    }
{{-   end }}
    return {{$enum}}::kUndefined;
}
{{- end -}}


{{- /* ------------------------------------------------------------------ */ -}}
{{-                         define "EnumOStream"                             -}}
{{- /* Implements the stream 'operator<<()' function to print the         */ -}}
{{- /* provided sem.Enum.                                                 */ -}}
{{- /* ------------------------------------------------------------------ */ -}}
{{- $enum := Eval "EnumName" $ -}}
    utils::StringStream& operator<<(utils::StringStream& out, {{$enum}} value) {
    switch (value) {
        case {{$enum}}::kUndefined:
            return out << "undefined";
{{-   range $entry := $.Entries }}
        case {{template "EnumCase" $entry}}:
            return out << "{{$entry.Name}}";
{{-   end }}
    }
    return out << "<unknown>";
}
{{- end -}}


{{- /* ------------------------------------------------------------------ */ -}}
{{-                        define "TestParsePrintEnum"                       -}}
{{- /* Implements unit tests for parsing and printing the provided        */ -}}
{{- /* sem.Enum argument.                                                 */ -}}
{{- /* ------------------------------------------------------------------ */ -}}
{{- $enum := Eval "EnumName" $ -}}
namespace parse_print_tests {

struct Case {
    const char* string;
    {{$enum}} value;
};

inline std::ostream& operator<<(std::ostream& out, Case c) {
    return out << "'" << std::string(c.string) << "'";
}

static constexpr Case kValidCases[] = {
{{-   range $entry := $.PublicEntries }}
    {"{{$entry.Name}}", {{template "EnumCase" $entry}}},
{{-   end }}
};

static constexpr Case kInvalidCases[] = {
{{-   $exclude := $.NameSet -}}
{{-   range $entry := $.PublicEntries }}
    {"{{Scramble $entry.Name $exclude}}", {{$enum}}::kUndefined},
    {"{{Scramble $entry.Name $exclude}}", {{$enum}}::kUndefined},
    {"{{Scramble $entry.Name $exclude}}", {{$enum}}::kUndefined},
{{-   end }}
};

using {{$enum}}ParseTest = testing::TestWithParam<Case>;

TEST_P({{$enum}}ParseTest, Parse) {
    const char* string = GetParam().string;
    {{$enum}} expect = GetParam().value;
    EXPECT_EQ(expect, Parse{{$enum}}(string));
}

INSTANTIATE_TEST_SUITE_P(ValidCases, {{$enum}}ParseTest, testing::ValuesIn(kValidCases));
INSTANTIATE_TEST_SUITE_P(InvalidCases, {{$enum}}ParseTest, testing::ValuesIn(kInvalidCases));

using {{$enum}}PrintTest = testing::TestWithParam<Case>;

TEST_P({{$enum}}PrintTest, Print) {
    {{$enum}} value = GetParam().value;
    const char* expect = GetParam().string;
    EXPECT_EQ(expect, utils::ToString(value));
}

INSTANTIATE_TEST_SUITE_P(ValidCases, {{$enum}}PrintTest, testing::ValuesIn(kValidCases));

}  // namespace parse_print_tests

{{- end -}}


{{- /* ------------------------------------------------------------------ */ -}}
{{-                     define "BenchmarkParseEnum"                          -}}
{{- /* Implements a micro-benchmark for parsing the provided sem.Enum     */ -}}
{{- /* argument.                                                          */ -}}
{{- /* ------------------------------------------------------------------ */ -}}
{{- $enum := Eval "EnumName" $ -}}
void {{$enum}}Parser(::benchmark::State& state) {
    const char* kStrings[] = {
{{-   $exclude := $.NameSet -}}
{{-   range $entry := $.PublicEntries }}
        "{{Scramble $entry.Name $exclude}}",
        "{{Scramble $entry.Name $exclude}}",
        "{{Scramble $entry.Name $exclude}}",
        "{{$entry.Name}}",
        "{{Scramble $entry.Name $exclude}}",
        "{{Scramble $entry.Name $exclude}}",
        "{{Scramble $entry.Name $exclude}}",
{{-   end }}
    };
    for (auto _ : state) {
        for (auto* str : kStrings) {
            auto result = Parse{{$enum}}(str);
            benchmark::DoNotOptimize(result);
        }
    }
} // NOLINT(readability/fn_size)

BENCHMARK({{$enum}}Parser);
{{- end -}}
