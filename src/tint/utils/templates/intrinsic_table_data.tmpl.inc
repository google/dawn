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

namespace {

using ConstEvalFunctionIndex = tint::core::intrinsic::ConstEvalFunctionIndex;
using IntrinsicInfo = tint::core::intrinsic::IntrinsicInfo;
using MatchState = tint::core::intrinsic::MatchState;
using Number = tint::core::intrinsic::Number;
using NumberMatcher = tint::core::intrinsic::NumberMatcher;
using NumberMatcherIndex = tint::core::intrinsic::NumberMatcherIndex;
using NumberMatcherIndicesIndex = tint::core::intrinsic::NumberMatcherIndicesIndex;
using OverloadFlag = tint::core::intrinsic::OverloadFlag;
using OverloadFlags = tint::core::intrinsic::OverloadFlags;
using OverloadIndex = tint::core::intrinsic::OverloadIndex;
using OverloadInfo = tint::core::intrinsic::OverloadInfo;
using ParameterIndex = tint::core::intrinsic::ParameterIndex;
using ParameterInfo = tint::core::intrinsic::ParameterInfo;
using StringStream = tint::StringStream;
using TemplateNumberIndex = tint::core::intrinsic::TemplateNumberIndex;
using TemplateNumberInfo = tint::core::intrinsic::TemplateNumberInfo;
using TemplateTypeIndex = tint::core::intrinsic::TemplateTypeIndex;
using TemplateTypeInfo = tint::core::intrinsic::TemplateTypeInfo;
using Type = tint::core::type::Type;
using TypeMatcher = tint::core::intrinsic::TypeMatcher;
using TypeMatcherIndex = tint::core::intrinsic::TypeMatcherIndex;
using TypeMatcherIndicesIndex = tint::core::intrinsic::TypeMatcherIndicesIndex;

template<size_t N>
using TemplateNumberMatcher = tint::core::intrinsic::TemplateNumberMatcher<N>;

template<size_t N>
using TemplateTypeMatcher = tint::core::intrinsic::TemplateTypeMatcher<N>;

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

constexpr TypeMatcherIndex kTypeMatcherIndices[] = {
{{- range $i, $idx := .TypeMatcherIndices }}
  /* [{{$i}}] */ TypeMatcherIndex({{$idx}}),
{{- end }}
};

static_assert(TypeMatcherIndex::CanIndex(kTypeMatcherIndices),
              "TypeMatcherIndex is not large enough to index kTypeMatcherIndices");

constexpr NumberMatcherIndex kNumberMatcherIndices[] = {
{{- range $i, $idx := .NumberMatcherIndices }}
  /* [{{$i}}] */ NumberMatcherIndex({{$idx}}),
{{- end }}
};

static_assert(NumberMatcherIndex::CanIndex(kNumberMatcherIndices),
              "NumberMatcherIndex is not large enough to index kNumberMatcherIndices");

constexpr ParameterInfo kParameters[] = {
{{- range $i, $p := .Parameters }}
  {
    /* [{{$i}}] */
    /* usage */ core::ParameterUsage::
{{-   if $p.Usage }}k{{PascalCase $p.Usage}}
{{-   else        }}kNone
{{-   end }},
    /* type_matcher_indices */ TypeMatcherIndicesIndex({{$p.TypeMatcherIndicesOffset}}),
    /* number_matcher_indices */
{{-   if ge $p.NumberMatcherIndicesOffset 0 }} NumberMatcherIndicesIndex({{$p.NumberMatcherIndicesOffset}})
{{-   else                                  }} NumberMatcherIndicesIndex(/* invalid */)
{{-   end }},
  },
{{- end }}
};

static_assert(ParameterIndex::CanIndex(kParameters),
              "ParameterIndex is not large enough to index kParameters");

constexpr TemplateTypeInfo kTemplateTypes[] = {
{{- range $i, $o := .TemplateTypes }}
  {
    /* [{{$i}}] */
    /* name */ "{{$o.Name}}",
    /* matcher_index */
{{-   if ge $o.MatcherIndex 0 }} TypeMatcherIndex({{$o.MatcherIndex}})
{{-   else                    }} TypeMatcherIndex(/* invalid */)
{{-   end }},
  },
{{- end }}
};

static_assert(TemplateTypeIndex::CanIndex(kTemplateTypes),
              "TemplateTypeIndex is not large enough to index kTemplateTypes");

constexpr TemplateNumberInfo kTemplateNumbers[] = {
{{- range $i, $o := .TemplateNumbers }}
  {
    /* [{{$i}}] */
    /* name */ "{{$o.Name}}",
    /* matcher_index */
{{-   if ge $o.MatcherIndex 0 }} NumberMatcherIndex({{$o.MatcherIndex}})
{{-   else                    }} NumberMatcherIndex(/* invalid */)
{{-   end }},
  },
{{- end }}
};

static_assert(TemplateNumberIndex::CanIndex(kTemplateNumbers),
              "TemplateNumberIndex is not large enough to index kTemplateNumbers");

{{- if .ConstEvalFunctions}}
{{/* newline */}}
constexpr core::constant::Eval::Function kConstEvalFunctions[] = {
{{-   range $i, $f := .ConstEvalFunctions }}
  /* [{{$i}}] */ &core::constant::Eval::{{template "OperatorName" $f}},
{{-   end }}
};

static_assert(ConstEvalFunctionIndex::CanIndex(kConstEvalFunctions),
              "ConstEvalFunctionIndex is not large enough to index kConstEvalFunctions");
{{- end}}

constexpr OverloadInfo kOverloads[] = {
{{- range $i, $o := .Overloads }}
  {
    /* [{{$i}}] */
    /* flags */ OverloadFlags(OverloadFlag::kIs{{Title $o.Kind}}
{{-   range $i, $u := $o.CanBeUsedInStage.List -}}
        , OverloadFlag::kSupports{{Title $u}}Pipeline
{{-   end }}
{{-   if $o.MustUse}}, OverloadFlag::kMustUse{{end}}
{{-   if $o.IsDeprecated}}, OverloadFlag::kIsDeprecated{{end -}}
    ),
    /* num_parameters */ {{$o.NumParameters}},
    /* num_template_types */ {{$o.NumTemplateTypes}},
    /* num_template_numbers */ {{$o.NumTemplateNumbers}},
    /* template_types */
{{-   if ge $o.TemplateTypesOffset 0 }} TemplateTypeIndex({{$o.TemplateTypesOffset}})
{{-   else                           }} TemplateTypeIndex(/* invalid */)
{{-   end }},
    /* template_numbers */
{{-   if ge $o.TemplateNumbersOffset 0 }} TemplateNumberIndex({{$o.TemplateNumbersOffset}})
{{-   else                             }} TemplateNumberIndex(/* invalid */)
{{-   end }},
    /* parameters */
{{-   if ge $o.ParametersOffset 0 }} ParameterIndex({{$o.ParametersOffset}})
{{-   else                        }} ParameterIndex(/* invalid */)
{{-   end }},
    /* return_type_matcher_indices */
{{-   if ge $o.ReturnTypeMatcherIndicesOffset 0 }} TypeMatcherIndicesIndex({{$o.ReturnTypeMatcherIndicesOffset}})
{{-   else                                      }} TypeMatcherIndicesIndex(/* invalid */)
{{-   end }},
    /* return_number_matcher_indices */
{{-   if ge $o.ReturnNumberMatcherIndicesOffset 0 }} NumberMatcherIndicesIndex({{$o.ReturnNumberMatcherIndicesOffset}})
{{-   else                                        }} NumberMatcherIndicesIndex(/* invalid */)
{{-   end }},
    /* const_eval_fn */
{{-   if ge $o.ConstEvalFunctionOffset 0 }} ConstEvalFunctionIndex({{$o.ConstEvalFunctionOffset}})
{{-   else                               }} ConstEvalFunctionIndex(/* invalid */)
{{-   end }},
  },
{{- end }}
};

static_assert(OverloadIndex::CanIndex(kOverloads),
              "OverloadIndex is not large enough to index kOverloads");

constexpr IntrinsicInfo kBuiltins[] = {
{{- range $i, $b := .Builtins }}
  {
    /* [{{$i}}] */
{{-   range $b.OverloadDescriptions }}
    /* {{.}} */
{{-   end }}
    /* num overloads */ {{$b.NumOverloads}},
    /* overloads */ OverloadIndex({{$b.OverloadsOffset}}),
  },
{{- end }}
};

{{- if .UnaryOperators}}
{{/* newline */}}
constexpr IntrinsicInfo kUnaryOperators[] = {
{{-   range $i, $o := .UnaryOperators }}
  {
    /* [{{$i}}] */
{{-     range $o.OverloadDescriptions }}
    /* {{.}} */
{{-     end }}
    /* num overloads */ {{$o.NumOverloads}},
    /* overloads */ OverloadIndex({{$o.OverloadsOffset}}),
  },
{{-   end }}
};
{{- end }}

{{- $unary_ops := Map }}
{{- range $i, $o := .UnaryOperators }}
{{-   $unary_ops.Put $o.Name true }}
constexpr uint8_t kUnaryOperator{{ template "OperatorName" $o.Name}} = {{$i}};
{{- end }}

{{- if .BinaryOperators}}
{{/* newline */}}
constexpr IntrinsicInfo kBinaryOperators[] = {
{{-   range $i, $o := .BinaryOperators }}
  {
    /* [{{$i}}] */
{{-     range $o.OverloadDescriptions }}
    /* {{.}} */
{{-     end }}
    /* num overloads */ {{$o.NumOverloads}},
    /* overloads */ OverloadIndex({{$o.OverloadsOffset}}),
  },
{{-   end }}
};
{{- end }}

{{- $bin_ops := Map }}
{{- range $i, $o := .BinaryOperators }}
{{-   $bin_ops.Put $o.Name true }}
constexpr uint8_t kBinaryOperator{{ template "OperatorName" $o.Name}} = {{$i}};
{{- end }}

{{- if .ConstructorsAndConverters}}
{{/* newline */}}
constexpr IntrinsicInfo kConstructorsAndConverters[] = {
{{-   range $i, $o := .ConstructorsAndConverters }}
  {
    /* [{{$i}}] */
{{-     range $o.OverloadDescriptions }}
    /* {{.}} */
{{-     end }}
    /* num overloads */ {{$o.NumOverloads}},
    /* overloads */ OverloadIndex({{$o.OverloadsOffset}}),
  },
{{-   end }}
};
{{- end }}

// clang-format on

}  // anonymous namespace

const core::intrinsic::TableData {{$.Name}}{
  /* template_types */ kTemplateTypes,
  /* template_numbers */ kTemplateNumbers,
  /* type_matcher_indices */ kTypeMatcherIndices,
  /* number_matcher_indices */ kNumberMatcherIndices,
  /* type_matchers */ kTypeMatchers,
  /* number_matchers */ kNumberMatchers,
  /* parameters */ kParameters,
  /* overloads */ kOverloads,
  /* const_eval_functions */ {{if .ConstEvalFunctions}}kConstEvalFunctions{{else}}Empty{{end}},
  /* ctor_conv */ {{if .ConstructorsAndConverters}}kConstructorsAndConverters{{else}}Empty{{end}},
  /* builtins */ kBuiltins,
{{- range $op := List "+" "-" "*" "/" "%" "^" "&" "|" "&&" "||" "==" "!=" "<" ">" "<=" ">=" "<<" ">>"}}
{{-   $N := Eval "OperatorName" $op }}
  /* binary '{{$op}}' */ {{if $bin_ops.Get .}}kBinaryOperators[kBinaryOperator{{$N}}]{{else}}tint::core::intrinsic::kNoOverloads{{end}},
{{- end}}
{{- range $op := List "!" "~" "-"}}
{{-   $N := Eval "OperatorName" $op }}
  /* unary '{{$op}}' */ {{if $unary_ops.Get .}}kUnaryOperators[kUnaryOperator{{$N}}]{{else}}tint::core::intrinsic::kNoOverloads{{end}},
{{- end}}
};

{{  end -}}
{{- end -}}


{{- /* ------------------------------------------------------------------ */ -}}
{{-                              define "Type"                               -}}
{{- /* ------------------------------------------------------------------ */ -}}
{{- $name := PascalCase .Name -}}
/// TypeMatcher for 'type {{.Name}}'
constexpr TypeMatcher k{{$name}}Matcher {
/* match */ [](MatchState& state, const Type* ty) -> const Type* {
{{- range .TemplateParams }}
{{-   template "DeclareLocalTemplateParam" . }}
{{- end  }}
    if (!Match{{$name}}(state, ty{{range .TemplateParams}}, {{.GetName}}{{end}})) {
      return nullptr;
    }
{{- range .TemplateParams }}
    {{.Name}} = {{ template "MatchTemplateParam" .}}({{.Name}});
    if ({{ template "IsTemplateParamInvalid" .}}) {
      return nullptr;
    }
{{- end  }}
    return Build{{$name}}(state, ty{{range .TemplateParams}}, {{.GetName}}{{end}});
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
{{- $name := PascalCase .Name -}}
/// TypeMatcher for 'match {{.Name}}'
constexpr TypeMatcher k{{$name}}Matcher {
/* match */ [](MatchState& state, const Type* ty) -> const Type* {
{{- range .PrecedenceSortedTypes }}
    if (Match{{PascalCase .Name}}(state, ty)) {
      return Build{{PascalCase .Name}}(state, ty);
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
    if (number.IsAny() || number.Value() == static_cast<uint32_t>(core::{{$enum}}::{{$entry}})) {
      return Number(static_cast<uint32_t>(core::{{$enum}}::{{$entry}}));
    }
    return Number::invalid;
  },
{{- else -}}
/* match */ [](MatchState&, Number number) -> Number {
    switch (static_cast<core::{{$enum}}>(number.Value())) {
{{-   range .Options }}
      case core::{{$enum}}::k{{PascalCase .Name}}:
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
{{-                          define "OperatorName"                           -}}
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
