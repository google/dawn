{{- /*
--------------------------------------------------------------------------------
Template file for use with tools/src/cmd/gen to generate the constant data that
is held in a core::intrinsic::TableData.

To update the generated file, run:
    ./tools/run gen

See:
* tools/src/cmd/gen for structures used by this template
* https://golang.org/pkg/text/template/ for documentation on the template syntax
--------------------------------------------------------------------------------
*/ -}}


{{- /* ------------------------------------------------------------------ */ -}}
{{-                              define "Data"                               -}}
{{- /* ------------------------------------------------------------------ */ -}}
{{- $I := $.Intrinsics -}}

namespace {{$.Namespace}} {
namespace {

using IntrinsicInfo = tint::core::intrinsic::TableData::IntrinsicInfo;
using MatcherIndex = tint::core::intrinsic::TableData::MatcherIndex;
using MatchState = tint::core::intrinsic::TableData::MatchState;
using Number = tint::core::intrinsic::TableData::Number;
using NumberMatcher = tint::core::intrinsic::TableData::NumberMatcher;
using OverloadFlag = tint::core::intrinsic::TableData::OverloadFlag;
using OverloadFlags = tint::core::intrinsic::TableData::OverloadFlags;
using OverloadInfo = tint::core::intrinsic::TableData::OverloadInfo;
using ParameterInfo = tint::core::intrinsic::TableData::ParameterInfo;
using StringStream = tint::StringStream;
using TemplateNumberInfo = tint::core::intrinsic::TableData::TemplateNumberInfo;
using TemplateTypeInfo = tint::core::intrinsic::TableData::TemplateTypeInfo;
using Type = tint::type::Type;
using TypeMatcher = tint::core::intrinsic::TableData::TypeMatcher;

template<size_t N>
using TemplateNumberMatcher = tint::core::intrinsic::TableData::TemplateNumberMatcher<N>;

template<size_t N>
using TemplateTypeMatcher = tint::core::intrinsic::TableData::TemplateTypeMatcher<N>;

static constexpr auto kNoMatcher = tint::core::intrinsic::TableData::kNoMatcher;

// clang-format off

{{  with $I.Sem -}}
{{    range .Types -}}
{{      template "Type" . }}
{{    end -}}
{{    range .TypeMatchers -}}
{{      template "TypeMatcher" . }}
{{    end -}}
{{    range .EnumMatchers -}}
{{      template "EnumMatcher" . }}
{{    end -}}
{{- end -}}

{{- with $I.Table -}}
{{- template "Matchers" $I }}

constexpr MatcherIndex kMatcherIndices[] = {
{{- range $i, $idx := .MatcherIndices }}
  /* [{{$i}}] */ {{$idx}},
{{- end }}
};

// Assert that the MatcherIndex is big enough to index all the matchers, plus
// kNoMatcher.
static_assert(static_cast<int>(sizeof(kMatcherIndices) / sizeof(kMatcherIndices[0])) <
              static_cast<int>(std::numeric_limits<MatcherIndex>::max() - 1),
              "MatcherIndex is not large enough to index kMatcherIndices");

constexpr ParameterInfo kParameters[] = {
{{- range $i, $p := .Parameters }}
  {
    /* [{{$i}}] */
    /* usage */ ParameterUsage::
{{-   if $p.Usage }}k{{PascalCase $p.Usage}}
{{-   else        }}kNone
{{-   end         }},
    /* matcher indices */ &kMatcherIndices[{{$p.MatcherIndicesOffset}}],
  },
{{- end }}
};

constexpr TemplateTypeInfo kTemplateTypes[] = {
{{- range $i, $o := .TemplateTypes }}
  {
    /* [{{$i}}] */
    /* name */ "{{$o.Name}}",
    /* matcher index */
{{-   if ge $o.MatcherIndex 0 }} {{$o.MatcherIndex}}
{{-   else                    }} kNoMatcher
{{-   end                     }},
  },
{{- end }}
};

constexpr TemplateNumberInfo kTemplateNumbers[] = {
{{- range $i, $o := .TemplateNumbers }}
  {
    /* [{{$i}}] */
    /* name */ "{{$o.Name}}",
    /* matcher index */
{{-   if ge $o.MatcherIndex 0 }} {{$o.MatcherIndex}}
{{-   else                    }} kNoMatcher
{{-   end                     }},
  },
{{- end }}
};

constexpr OverloadInfo kOverloads[] = {
{{- range $i, $o := .Overloads }}
  {
    /* [{{$i}}] */
    /* num parameters */ {{$o.NumParameters}},
    /* num template types */ {{$o.NumTemplateTypes}},
    /* num template numbers */ {{$o.NumTemplateNumbers}},
    /* template types */
{{-   if $o.TemplateTypesOffset }} &kTemplateTypes[{{$o.TemplateTypesOffset}}],
{{-   else                      }} nullptr,
{{-   end }}
    /* template numbers */
{{-   if $o.TemplateNumbersOffset }} &kTemplateNumbers[{{$o.TemplateNumbersOffset}}]
{{-   else                        }} nullptr
{{-   end }},
    /* parameters */ &kParameters[{{$o.ParametersOffset}}],
    /* return matcher indices */
{{-   if $o.ReturnMatcherIndicesOffset }} &kMatcherIndices[{{$o.ReturnMatcherIndicesOffset}}]
{{-   else                             }} nullptr
{{-   end }},
    /* flags */ OverloadFlags(OverloadFlag::kIs{{Title $o.Kind}}
{{-   range $i, $u := $o.CanBeUsedInStage.List -}}
        , OverloadFlag::kSupports{{Title $u}}Pipeline
{{-   end }}
{{-   if $o.MustUse}}, OverloadFlag::kMustUse{{end}}
{{-   if $o.IsDeprecated}}, OverloadFlag::kIsDeprecated{{end -}}
    ),
    /* const eval */
{{-   if $o.ConstEvalFunction }} {{template "ConstEvalFn" $o}},
{{-   else                    }} nullptr,
{{-   end }}
  },
{{- end }}
};

constexpr IntrinsicInfo kBuiltins[] = {
{{- range $i, $b := .Builtins }}
  {
    /* [{{$i}}] */
{{-   range $b.OverloadDescriptions }}
    /* {{.}} */
{{-   end }}
    /* num overloads */ {{$b.NumOverloads}},
    /* overloads */ &kOverloads[{{$b.OverloadsOffset}}],
  },
{{- end }}
};

constexpr IntrinsicInfo kUnaryOperators[] = {
{{- range $i, $o := .UnaryOperators }}
  {
    /* [{{$i}}] */
{{-   range $o.OverloadDescriptions }}
    /* {{.}} */
{{-   end }}
    /* num overloads */ {{$o.NumOverloads}},
    /* overloads */ &kOverloads[{{$o.OverloadsOffset}}],
  },
{{- end }}
};

{{- range $i, $o := .UnaryOperators }}
constexpr uint8_t kUnaryOperator{{ template "ExpandName" $o.Name}} = {{$i}};
{{- end }}

constexpr IntrinsicInfo kBinaryOperators[] = {
{{- range $i, $o := .BinaryOperators }}
  {
    /* [{{$i}}] */
{{-   range $o.OverloadDescriptions }}
    /* {{.}} */
{{-   end }}
    /* num overloads */ {{$o.NumOverloads}},
    /* overloads */ &kOverloads[{{$o.OverloadsOffset}}],
  },
{{- end }}
};

{{- range $i, $o := .BinaryOperators }}
constexpr uint8_t kBinaryOperator{{ template "ExpandName" $o.Name}} = {{$i}};
{{- end }}

constexpr IntrinsicInfo kConstructorsAndConverters[] = {
{{- range $i, $o := .ConstructorsAndConverters }}
  {
    /* [{{$i}}] */
{{-   range $o.OverloadDescriptions }}
    /* {{.}} */
{{-   end }}
    /* num overloads */ {{$o.NumOverloads}},
    /* overloads */ &kOverloads[{{$o.OverloadsOffset}}],
  },
{{- end }}
};

// clang-format on

}  // anonymous namespace

const TableData& {{$.Name}}() {
  static const TableData data{
    /* type_matchers */ kTypeMatchers,
    /* number_matchers */ kNumberMatchers,
    /* ctor_conv */ kConstructorsAndConverters,
    /* builtins */ kBuiltins,
    /* binary_plus */ kBinaryOperators[kBinaryOperatorPlus],
    /* binary_minus */ kBinaryOperators[kBinaryOperatorMinus],
    /* binary_star */ kBinaryOperators[kBinaryOperatorStar],
    /* binary_divide */ kBinaryOperators[kBinaryOperatorDivide],
    /* binary_modulo */ kBinaryOperators[kBinaryOperatorModulo],
    /* binary_xor */ kBinaryOperators[kBinaryOperatorXor],
    /* binary_and */ kBinaryOperators[kBinaryOperatorAnd],
    /* binary_or */ kBinaryOperators[kBinaryOperatorOr],
    /* binary_logical_and */ kBinaryOperators[kBinaryOperatorLogicalAnd],
    /* binary_logical_or */ kBinaryOperators[kBinaryOperatorLogicalOr],
    /* binary_equal */ kBinaryOperators[kBinaryOperatorEqual],
    /* binary_not_equal */ kBinaryOperators[kBinaryOperatorNotEqual],
    /* binary_less_than */ kBinaryOperators[kBinaryOperatorLessThan],
    /* binary_greater_than */ kBinaryOperators[kBinaryOperatorGreaterThan],
    /* binary_less_than_equal */ kBinaryOperators[kBinaryOperatorLessThanEqual],
    /* binary_greater_than_equal */ kBinaryOperators[kBinaryOperatorGreaterThanEqual],
    /* binary_shift_left */ kBinaryOperators[kBinaryOperatorShiftLeft],
    /* binary_shift_right */ kBinaryOperators[kBinaryOperatorShiftRight],
    /* unary_not */ kUnaryOperators[kUnaryOperatorNot],
    /* unary_complement */ kUnaryOperators[kUnaryOperatorComplement],
    /* unary_minus */ kUnaryOperators[kUnaryOperatorMinus],
  };
  return data;
}

}  // namespace {{$.Namespace}}

{{  end -}}
{{- end -}}


{{- /* ------------------------------------------------------------------ */ -}}
{{-                              define "Type"                               -}}
{{- /* ------------------------------------------------------------------ */ -}}
{{- $class := PascalCase .Name -}}
/// TypeMatcher for 'type {{.Name}}'
constexpr TypeMatcher k{{$class}}Matcher {
/* match */ [](MatchState& state, const Type* ty) -> const Type* {
{{- range .TemplateParams }}
{{-   template "DeclareLocalTemplateParam" . }}
{{- end  }}
    if (!match_{{TrimLeft .Name "_"}}(state, ty{{range .TemplateParams}}, {{.GetName}}{{end}})) {
      return nullptr;
    }
{{- range .TemplateParams }}
    {{.Name}} = {{ template "MatchTemplateParam" .}}({{.Name}});
    if ({{ template "IsTemplateParamInvalid" .}}) {
      return nullptr;
    }
{{- end  }}
    return build_{{TrimLeft .Name "_"}}(state{{range .TemplateParams}}, {{.GetName}}{{end}});
  },
/* string */ [](MatchState*{{if .TemplateParams}} state{{end}}) -> std::string {
{{- range .TemplateParams }}
{{-   template "DeclareLocalTemplateParamName" . }}
{{- end  }}

{{- if .DisplayName }}
    StringStream ss;
    ss{{range SplitDisplayName .DisplayName}} << {{.}}{{end}};
    return ss.str();
{{- else if .TemplateParams }}
    return "{{.Name}}<"{{template "AppendTemplateParamNames" .TemplateParams}} + ">";
{{- else }}
    return "{{.Name}}";
{{- end  }}
  }
};

{{  end -}}


{{- /* ------------------------------------------------------------------ */ -}}
{{-                          define "TypeMatcher"                            -}}
{{- /* ------------------------------------------------------------------ */ -}}
{{- $class := PascalCase .Name -}}
/// TypeMatcher for 'match {{.Name}}'
constexpr TypeMatcher k{{$class}}Matcher {
/* match */ [](MatchState& state, const Type* ty) -> const Type* {
{{- range .PrecedenceSortedTypes }}
    if (match_{{.Name}}(state, ty)) {
      return build_{{.Name}}(state);
    }
{{- end }}
    return nullptr;
  },
/* string */ [](MatchState*) -> std::string {
    StringStream ss;
    // Note: We pass nullptr to the TypeMatcher::String() functions, as 'matcher's do not support
    // template arguments, nor can they match sub-types. As such, they have no use for the MatchState.
    ss
{{- range .Types -}}
{{-   if      IsFirstIn . $.Types }} << k{{PascalCase .Name}}Matcher.string(nullptr)
{{-   else if IsLastIn  . $.Types }} << " or " << k{{PascalCase .Name}}Matcher.string(nullptr)
{{-   else                        }} << ", " << k{{PascalCase .Name}}Matcher.string(nullptr)
{{-   end -}}
{{- end -}};
    return ss.str();
  }
};
{{  end -}}


{{- /* ------------------------------------------------------------------ */ -}}
{{-                          define "EnumMatcher"                            -}}
{{- /* ------------------------------------------------------------------ */ -}}
{{- $class := PascalCase .Name -}}
{{- $enum := PascalCase .Enum.Name -}}
/// EnumMatcher for 'match {{.Name}}'
constexpr NumberMatcher k{{$class}}Matcher {
{{ if eq 1 (len .Options) -}}
{{-   $option := index .Options 0 }}
{{-   $entry := printf "k%v" (PascalCase $option.Name) -}}
/* match */ [](MatchState&, Number number) -> Number {
    if (number.IsAny() || number.Value() == static_cast<uint32_t>({{$enum}}::{{$entry}})) {
      return Number(static_cast<uint32_t>({{$enum}}::{{$entry}}));
    }
    return Number::invalid;
  },
{{- else -}}
/* match */ [](MatchState&, Number number) -> Number {
    switch (static_cast<{{$enum}}>(number.Value())) {
{{-   range .Options }}
      case {{$enum}}::k{{PascalCase .Name}}:
{{-   end }}
        return number;
      default:
        return Number::invalid;
    }
  },
{{- end }}
/* string */ [](MatchState*) -> std::string {
    return "
{{- range .Options -}}
{{-   if      IsFirstIn . $.Options }}{{.Name}}
{{-   else if IsLastIn  . $.Options }} or {{.Name}}
{{-   else                          }}, {{.Name}}
{{-   end -}}
{{- end -}}
";
  }
};
{{  end -}}

{{- /* ------------------------------------------------------------------ */ -}}
{{-                            define "Matchers"                             -}}
{{- /* ------------------------------------------------------------------ */ -}}
/// Type and number matchers
{{- $t_names := Map -}}
{{- $n_names := Map -}}
{{- range Iterate $.Sem.MaxTemplateTypes -}}
{{-   $name := printf "TemplateTypeMatcher<%v>::matcher" . -}}
{{-   $t_names.Put . $name }}
{{- end }}
{{- range Iterate $.Sem.MaxTemplateNumbers -}}
{{-   $name := printf "TemplateNumberMatcher<%v>::matcher" . -}}
{{-   $n_names.Put . $name }}
{{- end }}
{{- range $.Sem.Types -}}
{{-   $name := printf "k%vMatcher" (PascalCase .Name) -}}
{{-   $t_names.Put . $name }}
{{- end }}
{{- range $.Sem.TypeMatchers -}}
{{-   $name := printf "k%vMatcher" (PascalCase .Name) -}}
{{-   $t_names.Put . $name }}
{{- end }}
{{- range $.Sem.EnumMatchers -}}
{{-   $name := printf "k%vMatcher" (PascalCase .Name) -}}
{{-   $n_names.Put . $name }}
{{- end }}

/// The template types, types, and type matchers
constexpr TypeMatcher kTypeMatchers[] = {
{{- range $i, $m := $.Table.TMatchers }}
  /* [{{$i}}] */
{{-   if $m }} {{$t_names.Get $m}},
{{-   else  }} {{$t_names.Get $i}},
{{-   end   }}
{{- end }}
};

/// The template numbers, and number matchers
constexpr NumberMatcher kNumberMatchers[] = {
{{- range $i, $m := $.Table.NMatchers }}
  /* [{{$i}}] */
{{-   if $m }} {{$n_names.Get $m}},
{{-   else  }} {{$n_names.Get $i}},
{{-   end   }}
{{- end }}
};

{{- end -}}


{{- /* ------------------------------------------------------------------ */ -}}
{{-                     define "DeclareLocalTemplateParam"                   -}}
{{- /* ------------------------------------------------------------------ */ -}}
{{-   if      IsTemplateTypeParam . }}
  const Type* {{.Name}} = nullptr;
{{-   else if IsTemplateNumberParam . }}
  Number {{.Name}} = Number::invalid;
{{-   else if IsTemplateEnumParam . }}
  Number {{.Name}} = Number::invalid;
{{-   end -}}
{{- end -}}


{{- /* ------------------------------------------------------------------ */ -}}
{{-                   define "DeclareLocalTemplateParamName"                 -}}
{{- /* ------------------------------------------------------------------ */ -}}
{{-   if      IsTemplateTypeParam . }}
  const std::string {{.Name}} = state->TypeName();
{{-   else if IsTemplateNumberParam . }}
  const std::string {{.Name}} = state->NumName();
{{-   else if IsTemplateEnumParam . }}
  const std::string {{.Name}} = state->NumName();
{{-   end -}}
{{- end -}}


{{- /* ------------------------------------------------------------------ */ -}}
{{-                       define "MatchTemplateParam"                        -}}
{{- /* ------------------------------------------------------------------ */ -}}
{{-   if      IsTemplateTypeParam . -}}
  state.Type
{{-   else if IsTemplateNumberParam . -}}
  state.Num
{{-   else if IsTemplateEnumParam . -}}
  state.Num
{{-   end -}}
{{- end -}}


{{- /* ------------------------------------------------------------------ */ -}}
{{-                       define "IsTemplateParamInvalid"                    -}}
{{- /* ------------------------------------------------------------------ */ -}}
{{-   if      IsTemplateTypeParam . -}}
  {{.Name}} == nullptr
{{-   else if IsTemplateNumberParam . -}}
  !{{.Name}}.IsValid()
{{-   else if IsTemplateEnumParam . -}}
  !{{.Name}}.IsValid()
{{-   end -}}
{{- end -}}


{{- /* ------------------------------------------------------------------ */ -}}
{{-                      define "AppendTemplateParamNames"                   -}}
{{- /* ------------------------------------------------------------------ */ -}}
{{-   range $i, $ := . -}}
{{-     if $i }} + ", " + {{.Name}}
{{-     else }} + {{.Name}}
{{-     end -}}
{{-   end -}}
{{- end -}}


{{- /* ------------------------------------------------------------------ */ -}}
{{-                           define "ExpandName"                            -}}
{{- /* ------------------------------------------------------------------ */ -}}
{{-        if eq . "<<" -}}ShiftLeft
{{-   else if eq . "&"  -}}And
{{-   else if eq . "|"  -}}Or
{{-   else if eq . "^"  -}}Xor
{{-   else if eq . "&&" -}}LogicalAnd
{{-   else if eq . "||" -}}LogicalOr
{{-   else if eq . "==" -}}Equal
{{-   else if eq . "!"  -}}Not
{{-   else if eq . "!=" -}}NotEqual
{{-   else if eq . "~"  -}}Complement
{{-   else if eq . "<"  -}}LessThan
{{-   else if eq . ">"  -}}GreaterThan
{{-   else if eq . "<=" -}}LessThanEqual
{{-   else if eq . ">=" -}}GreaterThanEqual
{{-   else if eq . "<<" -}}ShiftLeft
{{-   else if eq . ">>" -}}ShiftRight
{{-   else if eq . "+"  -}}Plus
{{-   else if eq . "-"  -}}Minus
{{-   else if eq . "*"  -}}Star
{{-   else if eq . "/"  -}}Divide
{{-   else if eq . "%"  -}}Modulo
{{-   else              -}}{{.}}
{{-   end -}}
{{- end -}}


{{- /* ------------------------------------------------------------------ */ -}}
{{-                           define "ConstEvalFn"                           -}}
{{- /* ------------------------------------------------------------------ */ -}}
  &constant::Eval::
{{-   if eq .Kind "operator" -}}Op{{end -}}
  {{template "ExpandName" .ConstEvalFunction}}
{{- end -}}
