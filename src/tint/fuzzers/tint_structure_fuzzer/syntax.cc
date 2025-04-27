#include "syntax.h"
#include <array>
#include <cassert>
#include <cctype>
#include <cmath>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <sstream>
#include <string_view>
#include <utility>
#include <unordered_map>
#include "src/tint/fuzzers/random_generator.h"
#include "src/tint/utils/math/crc32.h"

#define INLINE __attribute__((always_inline))

namespace tint::fuzzers::structure_fuzzer {

namespace {

struct Context {
    std::unordered_map<std::string, std::string> vars; 
    RandomGenerator& gen;
    int nextVarId = 0;
    std::vector<uint8_t> storedSubtree;

    explicit Context(RandomGenerator& generator) : gen(generator) {}

    std::string createVariable(const std::string& type) {
        std::string name = "v" + std::to_string(nextVarId++);
        vars[name] = type;
        return name;
    }

    std::string getRandomVariable() {
        if (vars.empty()) return "";
        auto it = vars.begin();
        std::advance(it, gen.GetUInt32(vars.size()));
        return it->first;
    }

    bool shouldUseVariable() {
        return !vars.empty() && gen.GetUInt32(2) == 0;
    }
};

struct VariableContext {
    std::unordered_map<std::string, std::string> vars;  
    int nextVarId = 0;

    std::string createVariable(const std::string& type) {
        std::string name = "v" + std::to_string(nextVarId++);
        vars[name] = type;
        return name;
    }

    std::string getRandomVariable(RandomGenerator& gen) {
        if (vars.empty()) return "";
        auto it = vars.begin();
        std::advance(it, gen.GetUInt32(vars.size()));
        return it->first;
    }
};

struct ByteInputRnd {
    RandomGenerator& gen;
    Context& ctx;

    ByteInputRnd(RandomGenerator& g, Context& c) : gen(g), ctx(c) {}

    uint8_t byteTerm() { return gen.GetUInt32(256); }

    uint8_t byte() { return 0; }

    uint32_t range(uint32_t limit, bool repeat) INLINE {
        if (repeat) {
            return 0;
        }
        if (limit == 1) {
            return 0;
        }
        uint32_t x = gen.GetUInt32(INT32_MAX);
        float f = static_cast<float>(x) / static_cast<float>(INT32_MAX);
        f = std::pow(f, 2.2f);
        return std::clamp(static_cast<uint32_t>(f * limit), 0u, limit - 1);
    }
};

static void printHex(const uint8_t* data, size_t size) {
    for (size_t i = 0; i < size; ++i) {
        printf("%02X", data[i]);
    }
    printf("\n");
}

struct ByteInput {
    const uint8_t* data;
    size_t size;
    size_t used;
    bool acceptTruncated;
    Context& ctx;

    ByteInput(const uint8_t* d, size_t s, size_t u, bool a, Context& c)
        : data(d), size(s), used(u), acceptTruncated(a), ctx(c) {}

    void reset() { used = 0; }

    uint8_t byte() {
        uint8_t result = 0;
        if (used < size) {
            result = data[used];
            ++used;
        } else {
            ++used;
        }
        return result;
    }

    uint8_t byteTerm() {
        if (used < size) {
            return byte();
        }
        return ctx.gen.GetUInt32(256);
    }

    uint32_t range(uint32_t limit, bool repeat) INLINE {
        if (limit == 1) {
            return 0;
        }
        assert(limit <= 256);
        return std::min(static_cast<uint32_t>(byte()), limit - 1u);
    }
};


struct ByteOutput {
    std::vector<uint8_t> out;
    void push(uint8_t val) { out.push_back(val); }
};

struct ByteOutputNull {
    void push(uint8_t val) {}
};

struct TextOutput {
    std::stringstream buffer;
    char last = 0;
    Context& ctx;

    explicit TextOutput(Context& c) : ctx(c) {}

    void raw(std::string_view s) INLINE {
        if (s.empty()) {
            return;
        }
        if (bool(std::isalnum(last)) == bool(std::isalnum(s.front()))) {
            buffer << " ";
        }
        buffer << s;
        last = s.back();
    }

    void ident(int n, std::string_view prefix = "x") INLINE {
        raw(std::string(prefix) + std::to_string(n));
    }
};

enum class NodeId {
    translation_unit = 0,
    additive_operator,
    expression_list,
    argument_expression_list,
    assignment_statement,
    attribute,
    bitwise_expression_post_unary_expression_1,
    bitwise_expression_post_unary_expression_2,
    bitwise_expression_post_unary_expression_3,
    bitwise_expression_post_unary_expression,
    bool_literal,
    case_selector,
    component_or_swizzle_specifier,
    compound_assignment_operator,
    compound_statement,
    core_lhs_expression,
    decimal_float_literal,
    decimal_int_literal,
    diagnostic_control,
    expression_1,
    expression_2,
    expression,
    float_literal,
    for_init,
    for_update,
    assign_expression,
    comma_param,
    global_decl_1,
    return_type,
    comma_struct_field,
    global_decl,
    comma_ident_pattern_token_1,
    comma_ident_pattern_token_2,
    global_directive,
    global_value_decl,
    hex_float_literal,
    int_literal,
    lhs_expression,
    literal,
    member_ident,
    multiplicative_operator,
    optionally_typed_ident_1,
    optionally_typed_ident,
    param,
    primary_expression,
    relational_expression_post_unary_expression,
    multiplicative_operator_unary_expression,
    shift_expression_post_unary_expression_1,
    shift_expression_post_unary_expression,
    elseif_statement,
    else_statement,
    breakif_statement,
    continuing_statement,
    statement,
    switch_clause_1,
    switch_clause,
    swizzle_name,
    template_arg_expression,
    comma_expression,
    unary_expression,
    expression_list_angle,
    variable_decl,
    variable_or_value_statement,
    variable_updating_statement,

    last = variable_updating_statement,
};

using GenerateFn0 = std::function<void(TextOutput&)>;
using GenerateFn = std::function<void(uint8_t, TextOutput&)>;

GenerateFn0 emit(std::string_view sv);

enum class IdentType {
    Type,
    UserType,
    Function,
    UserFunction,
    Variable,
    Other,
};

GenerateFn ident(IdentType type);

enum class KeywordList {
    diagnostic_severity,
    requires_extensions,
    address_space,
};

GenerateFn keywords(KeywordList list);

void floatLiteral(TextOutput&);
void floatHexLiteral(TextOutput&);
void decimalLiteral(TextOutput&);
void hexLiteral(TextOutput&);

struct Subnode {
    Subnode(void (*fn)(uint8_t, TextOutput&), char mod = 0) : Subnode(GenerateFn(fn), mod) {}
    Subnode(void (*fn)(TextOutput&), char mod = 0) : Subnode(GenerateFn0(fn), mod) {}
    Subnode(GenerateFn fn, char mod = 0) : content(fn), mod(mod) {}
    Subnode(GenerateFn0 fn, char mod = 0) : content(fn), mod(mod) {}
    Subnode(NodeId node, char mod = 0) : content(node), mod(mod) {}
    std::variant<GenerateFn0, GenerateFn, NodeId> content;
    char mod = 0;
};

struct Modifier {
    char v;
};

constexpr inline int maxDepth = 16;
constexpr inline int maxRepeats = 5;

constexpr inline Modifier many{'*'};
constexpr inline Modifier optional{'?'};

inline Subnode operator|(Subnode subnode, Modifier mod) {
    subnode.mod = mod.v;
    return subnode;
}

struct Node : public std::vector<std::vector<Subnode>> {
    Node(std::initializer_list<std::vector<Subnode>> list) INLINE
        : std::vector<std::vector<Subnode>>(list) {
        assert(this->size() >= 1);
    }
};

constexpr inline int limit = 9;

const auto& nodes() {
    static std::array instance{
        // translation_unit:
        Node{
            {NodeId::global_decl | many},
        },
        // additive_operator:
        Node{
            {emit("+")},
            {emit("-")},
        },
        // expression_list:
        Node{
            {NodeId::expression, NodeId::comma_expression | many, emit(",") | optional},
        },
        // argument_expression_list:
        Node{
            {emit("("), NodeId::expression_list | optional, emit(")")},
        },
        // assignment_statement:
        Node{
            {NodeId::compound_assignment_operator},
            {emit("=")},
        },
        // attribute:
        Node{
            {emit("@"), emit("compute")},
            {emit("@"), emit("const")},
            {emit("@"), emit("fragment")},
            {emit("@"), emit("interpolate"), emit("("), ident(IdentType::Other),
             emit(",") | optional, emit(")")},
            {emit("@"), emit("interpolate"), emit("("), ident(IdentType::Other), emit(","),
             ident(IdentType::Other), emit(",") | optional, emit(")")},
            {emit("@"), emit("invariant")},
            {emit("@"), emit("must_use")},
            {emit("@"), emit("vertex")},
            {emit("@"), emit("workgroup_size"), emit("("), NodeId::expression, emit(",") | optional,
             emit(")")},
            {emit("@"), emit("workgroup_size"), emit("("), NodeId::expression, emit(","),
             NodeId::expression, emit(",") | optional, emit(")")},
            {emit("@"), emit("workgroup_size"), emit("("), NodeId::expression, emit(","),
             NodeId::expression, emit(","), NodeId::expression, emit(",") | optional, emit(")")},
            {emit("@"), emit("align"), emit("("), NodeId::expression, emit(",") | optional,
             emit(")")},
            {emit("@"), emit("binding"), emit("("), NodeId::expression, emit(",") | optional,
             emit(")")},
            {emit("@"), emit("blend_src"), emit("("), NodeId::expression, emit(",") | optional,
             emit(")")},
            {emit("@"), emit("builtin"), emit("("), ident(IdentType::Other), emit(",") | optional,
             emit(")")},
            {emit("@"), emit("diagnostic"), NodeId::diagnostic_control},
            {emit("@"), emit("group"), emit("("), NodeId::expression, emit(",") | optional,
             emit(")")},
            {emit("@"), emit("id"), emit("("), NodeId::expression, emit(",") | optional, emit(")")},
            {emit("@"), emit("location"), emit("("), NodeId::expression, emit(",") | optional,
             emit(")")},
            {emit("@"), emit("size"), emit("("), NodeId::expression, emit(",") | optional,
             emit(")")},
        },
        // bitwise_expression_post_unary_expression_1:
        Node{
            {emit("&"), NodeId::unary_expression},
        },
        // bitwise_expression_post_unary_expression_2:
        Node{
            {emit("^"), NodeId::unary_expression},
        },
        // bitwise_expression_post_unary_expression_3:
        Node{
            {emit("|"), NodeId::unary_expression},
        },
        // bitwise_expression_post_unary_expression:
        Node{
            {emit("&"), NodeId::unary_expression,
             NodeId::bitwise_expression_post_unary_expression_1 | many},
            {emit("^"), NodeId::unary_expression,
             NodeId::bitwise_expression_post_unary_expression_2 | many},
            {emit("|"), NodeId::unary_expression,
             NodeId::bitwise_expression_post_unary_expression_3 | many},
        },
        // bool_literal:
        Node{
            {emit("false")},
            {emit("true")},
        },
        // case_selector:
        Node{
            {NodeId::expression},
            {emit("default")},
        },
        // component_or_swizzle_specifier:
        Node{
            {emit("."), NodeId::member_ident, NodeId::component_or_swizzle_specifier | optional},
            {emit("."), NodeId::swizzle_name, NodeId::component_or_swizzle_specifier | optional},
            {emit("["), NodeId::expression, emit("]"),
             NodeId::component_or_swizzle_specifier | optional},
        },
        // compound_assignment_operator:
        Node{
            {emit("<<=")},
            {emit(">>=")},
            {emit("%=")},
            {emit("&=")},
            {emit("*=")},
            {emit("+=")},
            {emit("-=")},
            {emit("/=")},
            {emit("^=")},
            {emit("|=")},
        },
        // compound_statement:
        Node{
            {NodeId::attribute | many, emit("{"), NodeId::statement | many, emit("}")},
        },
        // core_lhs_expression:
        Node{
            {ident(IdentType::Other)},
            {emit("("), NodeId::lhs_expression, emit(")")},
        },
        // decimal_float_literal:
        Node{
            {&floatLiteral},
        },
        // decimal_int_literal:
        Node{
            {&decimalLiteral},
        },
        // diagnostic_control:
        Node{
            {emit("("), keywords(KeywordList::diagnostic_severity), emit(","),
             emit("derivative_uniformity"), emit(",") | optional, emit(")")},
        },
        // expression_1:
        Node{
            {emit("&&"), NodeId::unary_expression,
             NodeId::relational_expression_post_unary_expression},
        },
        // expression_2:
        Node{
            {emit("||"), NodeId::unary_expression,
             NodeId::relational_expression_post_unary_expression},
        },
        // expression:
        Node{
            {NodeId::unary_expression, NodeId::bitwise_expression_post_unary_expression},
            {NodeId::unary_expression, NodeId::relational_expression_post_unary_expression},
            {NodeId::unary_expression, NodeId::relational_expression_post_unary_expression,
             emit("&&"), NodeId::unary_expression,
             NodeId::relational_expression_post_unary_expression, NodeId::expression_1 | many},
            {NodeId::unary_expression, NodeId::relational_expression_post_unary_expression,
             emit("||"), NodeId::unary_expression,
             NodeId::relational_expression_post_unary_expression, NodeId::expression_2 | many},
        },
        // float_literal:
        Node{
            {floatLiteral},
            {floatHexLiteral},
        },
        // for_init:
        Node{
            {ident(IdentType::Variable), NodeId::argument_expression_list},
            {NodeId::variable_or_value_statement},
            {NodeId::variable_updating_statement},
        },
        // for_update:
        Node{
            {ident(IdentType::Variable), NodeId::argument_expression_list},
            {NodeId::variable_updating_statement},
        },
        // assign_expression:
        Node{
            {emit("="), NodeId::expression},
        },
        // comma_param:
        Node{
            {emit(","), NodeId::param},
        },
        // global_decl_1:
        Node{
            {NodeId::attribute | many, ident(IdentType::Variable), emit(":"),
             ident(IdentType::Type), NodeId::comma_param | many, emit(",") | optional},
        },
        // return_type:
        Node{
            {emit("->"), NodeId::attribute | many, ident(IdentType::Type)},
        },
        // comma_struct_field:
        Node{
            {emit(","), NodeId::attribute | many, NodeId::member_ident, emit(":"),
             ident(IdentType::Type)},
        },
        // global_decl:
        Node{
            {NodeId::attribute | many, emit("fn"), ident(IdentType::UserFunction), emit("("),
             NodeId::global_decl_1 | optional, emit(")"), NodeId::return_type | optional,
             NodeId::attribute | many, emit("{"), NodeId::statement | many, emit("}")},
            {NodeId::attribute | many, emit("var"), NodeId::expression_list_angle | optional,
             NodeId::optionally_typed_ident, NodeId::assign_expression | optional, emit(";")},
            {NodeId::global_value_decl, emit(";")},
            {emit(";")},
            {emit("struct"), ident(IdentType::UserType), emit("{"), NodeId::attribute | many,
             NodeId::member_ident, emit(":"), ident(IdentType::Type),
             NodeId::comma_struct_field | many, emit(",") | optional, emit("}")},
            {emit("const_assert"), NodeId::expression, emit(";")},
            {emit("alias"), ident(IdentType::UserType), emit("="), ident(IdentType::Type),
             emit(";")},
        },
        // comma_ident_pattern_token_1:
        Node{
            {emit(","), emit("f16")},
        },
        // comma_ident_pattern_token_2:
        Node{
            {emit(","), keywords(KeywordList::requires_extensions)},
        },
        // global_directive:
        Node{
            {emit("diagnostic"), emit("("), keywords(KeywordList::diagnostic_severity), emit(","),
             emit("derivative_uniformity"), emit(",") | optional, emit(")"), emit(";")},
            {emit("enable"), emit("f16"), NodeId::comma_ident_pattern_token_1 | many,
             emit(",") | optional, emit(";")},
            {emit("requires"), keywords(KeywordList::requires_extensions),
             NodeId::comma_ident_pattern_token_2 | many, emit(",") | optional, emit(";")},
        },
        // global_value_decl:
        Node{
            {NodeId::attribute | many, emit("override"), NodeId::optionally_typed_ident,
             NodeId::assign_expression | optional},
            {emit("const"), NodeId::optionally_typed_ident, NodeId::assign_expression},
        },
        // hex_float_literal:
        Node{
            {&floatHexLiteral},
        },
        // int_literal:
        Node{
            {&decimalLiteral},
            {hexLiteral},
        },
        // lhs_expression:
        Node{
            {NodeId::core_lhs_expression, NodeId::component_or_swizzle_specifier | optional},
            {emit("&"), NodeId::lhs_expression},
            {emit("*"), NodeId::lhs_expression},
        },
        // literal:
        Node{
            {NodeId::int_literal},
            {NodeId::float_literal},
            {NodeId::bool_literal},
        },
        // member_ident:
        Node{
            {ident(IdentType::Variable)},
        },
        // multiplicative_operator:
        Node{
            {emit("*")},
            {emit("/")},
            {emit("%")},
        },
        // optionally_typed_ident_1:
        Node{
            {emit(":"), ident(IdentType::Type)},
        },
        // optionally_typed_ident:
        Node{
            {ident(IdentType::Variable), NodeId::optionally_typed_ident_1 | optional},
        },
        // param:
        Node{
            {NodeId::attribute | many, ident(IdentType::Variable), emit(":"),
             ident(IdentType::Type)},
        },
        // primary_expression:
        Node{
            {NodeId::literal},
            {ident(IdentType::Variable)},
            {ident(IdentType::Function), NodeId::argument_expression_list},
            {emit("("), NodeId::expression, emit(")")},
            {ident(IdentType::Type), NodeId::argument_expression_list},
        },
        // relational_expression_post_unary_expression:
        Node{
            {NodeId::shift_expression_post_unary_expression, emit("=="), NodeId::unary_expression,
             NodeId::shift_expression_post_unary_expression},
            {NodeId::shift_expression_post_unary_expression, emit("!="), NodeId::unary_expression,
             NodeId::shift_expression_post_unary_expression},
            {NodeId::shift_expression_post_unary_expression},
            {NodeId::shift_expression_post_unary_expression, emit(">"), NodeId::unary_expression,
             NodeId::shift_expression_post_unary_expression},
            {NodeId::shift_expression_post_unary_expression, emit(">="), NodeId::unary_expression,
             NodeId::shift_expression_post_unary_expression},
            {NodeId::shift_expression_post_unary_expression, emit("<"), NodeId::unary_expression,
             NodeId::shift_expression_post_unary_expression},
            {NodeId::shift_expression_post_unary_expression, emit("<="), NodeId::unary_expression,
             NodeId::shift_expression_post_unary_expression},
        },
        // multiplicative_operator_unary_expression:
        Node{
            {NodeId::multiplicative_operator, NodeId::unary_expression},
        },
        // shift_expression_post_unary_expression_1:
        Node{
            {NodeId::additive_operator, NodeId::unary_expression,
             NodeId::multiplicative_operator_unary_expression | many},
        },
        // shift_expression_post_unary_expression:
        Node{
            {NodeId::multiplicative_operator_unary_expression | many,
             NodeId::shift_expression_post_unary_expression_1 | many},
            {emit("<<"), NodeId::unary_expression},
            {emit(">>"), NodeId::unary_expression},
        },
        // elseif_statement:
        Node{
            {emit("else"), emit("if"), NodeId::expression, NodeId::compound_statement},
        },
        // else_statement:
        Node{
            {emit("else"), NodeId::compound_statement},
        },
        // breakif_statement:
        Node{
            {emit("break"), emit("if"), NodeId::expression, emit(";")},
        },
        // continuing_statement:
        Node{
            {emit("continuing"), NodeId::attribute | many, emit("{"), NodeId::statement | many,
             NodeId::breakif_statement | optional, emit("}")},
        },
        // statement:
        Node{
            {emit("return"), NodeId::expression, emit(";")},
            {NodeId::variable_or_value_statement, emit(";")},
            {NodeId::variable_updating_statement, emit(";")},
            {NodeId::attribute | many, emit("if"), NodeId::expression, NodeId::compound_statement,
             NodeId::elseif_statement | many, NodeId::else_statement | optional},
            {NodeId::attribute | many, emit("for"), emit("("), NodeId::for_init | optional,
             emit(";"), NodeId::expression | optional, emit(";"), NodeId::for_update | optional,
             emit(")"), NodeId::compound_statement},
            {emit("return"), emit(";")},
            {NodeId::attribute | many, emit("loop"), NodeId::attribute | many, emit("{"),
             NodeId::statement | many, NodeId::continuing_statement | optional, emit("}")},
            {NodeId::attribute | many, emit("switch"), NodeId::expression, NodeId::attribute | many,
             emit("{"), NodeId::switch_clause | many, emit("}")},
            {NodeId::attribute | many, emit("while"), NodeId::expression,
             NodeId::compound_statement},
            {NodeId::compound_statement},
            {ident(IdentType::Type), NodeId::argument_expression_list, emit(";")},
            {emit("break"), emit(";")},
            {emit("continue"), emit(";")},
            {emit("const_assert"), NodeId::expression, emit(";")},
            {emit("discard"), emit(";")},
            {emit(";")},
        },
        // switch_clause_1:
        Node{
            {emit(","), NodeId::case_selector},
        },
        // switch_clause:
        Node{
            {emit("case"), NodeId::case_selector, NodeId::switch_clause_1 | many,
             emit(",") | optional, emit(":") | optional, NodeId::compound_statement},
            {emit("default"), emit(":") | optional, NodeId::compound_statement},
        },
        // swizzle_name:
        Node{
            {emit("x")},
            {emit("xy")},
            {emit("xyz")},
            {emit("xyzw")},
            {emit("r")},
            {emit("rg")},
            {emit("rgb")},
            {emit("rgba")},
            {emit("x")},
            {emit("xx")},
            {emit("xxx")},
            {emit("xxxx")},
        },
        // template_arg_expression:
        Node{
            {NodeId::expression},
        },
        // comma_expression:
        Node{
            {emit(","), NodeId::expression},
        },
        // unary_expression:
        Node{
            {NodeId::primary_expression, NodeId::component_or_swizzle_specifier | optional},
            {emit("!"), NodeId::unary_expression},
            {emit("&"), NodeId::unary_expression},
            {emit("*"), NodeId::unary_expression},
            {emit("-"), NodeId::unary_expression},
            {emit("~"), NodeId::unary_expression},
        },
        // expression_list_angle:
        Node{
            {emit("<"), keywords(KeywordList::address_space), emit(">")},
        },
        // variable_decl:
        Node{
            {emit("var"), NodeId::expression_list_angle | optional, NodeId::optionally_typed_ident},
        },
        // variable_or_value_statement:
        Node{
            // {NodeId::variable_decl},
            {NodeId::variable_decl, NodeId::assign_expression},
            {emit("const"), NodeId::optionally_typed_ident, NodeId::assign_expression},
            {emit("let"), NodeId::optionally_typed_ident, NodeId::assign_expression},
        },
        // variable_updating_statement:
        Node{
            {NodeId::lhs_expression, NodeId::assign_expression},
            {NodeId::lhs_expression, NodeId::compound_assignment_operator, NodeId::expression},
            {NodeId::lhs_expression, emit("++")},
            {NodeId::lhs_expression, emit("--")},
            {emit("_"), NodeId::assign_expression},
        },
    };
    static_assert(instance.size() == static_cast<size_t>(NodeId::last) + 1);
    return instance;
}

GenerateFn0 emit(std::string_view string) {
    return [string](TextOutput& out) INLINE { out.raw(string); };
}

GenerateFn keywords(KeywordList list) {
    return [list](uint8_t value, TextOutput& out) {
        switch (list) {
            case KeywordList::diagnostic_severity: {
                out.raw(std::array{"off", "error", "warning", "info"}[value % 4]);
                break;
            }
            case KeywordList::requires_extensions: {
                out.raw(std::array{"packed_4x8_integer_dot_product",
                                   "pointer_composite_access"}[value % 2]);
                break;
            }
            case KeywordList::address_space: {
                out.raw(std::array{"function", "private", "workgroup", "uniform",
                                   "storage"}[value % 5]);
                break;
            }
        }
    };
}

GenerateFn ident(IdentType type) {
    return [type](uint8_t value, TextOutput& out) INLINE {
        constexpr std::string_view funcs[] = {
            "abs",
            "acos",
            "acosh",
            "asin",
            "asinh",
            "atan",
            "atanh",
            "atan2",
            "ceil",
            "clamp",
            "cos",
            "cosh",
            "countLeadingZeros",
            "countOneBits",
            "countTrailingZeros",
            "cross",
            "degrees",
            "determinant",
            "distance",
            "dot",
            "dot4U8Packed",
            "dot4I8Packed",
            "exp",
            "exp2",
            "extractBits",
            "faceForward",
            "firstLeadingBit",
            "firstTrailingBit",
            "floor",
            "fma",
            "fract",
            "frexp",
            "insertBits",
            "inverseSqrt",
            "ldexp",
            "length",
            "log",
            "log2",
            "max",
            "min",
            "mix",
            "modf",
            "normalize",
            "pow",
            "radians",
            "reflect",
            "refract",
            "reverseBits",
            "round",
            "saturate",
            "sign",
            "sin",
            "sinh",
            "smoothstep",
            "sqrt",
            "step",
            "tan",
            "tanh",
            "transpose",
            "trunc",
            "dpdx",
            "dpdxCoarse",
            "dpdxFine",
            "dpdy",
            "dpdyCoarse",
            "dpdyFine",
            "fwidth",
            "fwidthCoarse",
            "fwidthFine",
            "textureDimensions",
            "textureGather",
            "textureGatherCompare",
            "textureLoad",
            "textureNumLayers",
            "textureNumLevels",
            "textureNumSamples",
            "textureSample",
            "textureSampleBias",
            "textureSampleCompare",
            "textureSampleCompareLevel",
            "textureSampleGrad",
            "textureSampleLevel",
            "textureSampleBaseClampToEdge",
            "textureStore",
            "atomicLoad",
            "atomicStore",
            "atomicAdd",
            "atomicSub",
            "atomicMax",
            "atomicMin",
            "atomicAnd",
            "atomicOr",
            "atomicXor",
            "atomicExchange",
            "atomicCompareExchangeWeak",
            "pack4x8snorm",
            "pack4x8unorm",
            "pack4xI8",
            "pack4xU8",
            "pack4xI8Clamp",
            "pack4xU8Clamp",
            "pack2x16snorm",
            "pack2x16unorm",
            "pack2x16float",
            "unpack4x8snorm",
            "unpack4x8unorm",
            "unpack4xI8",
            "unpack4xU8",
            "unpack2x16snorm",
            "unpack2x16unorm",
            "unpack2x16float",
            "storageBarrier",
            "textureBarrier",
            "workgroupBarrier",
            "workgroupUniformLoad",
        };
        constexpr std::string_view types[] = {
            "bool",
            "vec2<bool>",
            "vec3<bool>",
            "vec4<bool>",
            "u32",
            "vec2<u32>",
            "vec3<u32>",
            "vec4<u32>",
            "i32",
            "vec2<i32>",
            "vec3<i32>",
            "vec4<i32>",
            "f32",
            "vec2<f32>",
            "vec3<f32>",
            "vec4<f32>",
            "mat2x2<f32>",
            "mat2x3<f32>",
            "mat2x4<f32>",
            "mat3x2<f32>",
            "mat3x3<f32>",
            "mat3x4<f32>",
            "mat4x2<f32>",
            "mat4x3<f32>",
            "mat4x4<f32>",
            "array<bool, 1>",
            "array<bool, 16>",
            "array<u32, 1>",
            "array<u32, 16>",
            "array<i32, 1>",
            "array<i32, 16>",
            "array<f32, 1>",
            "array<f32, 16>",
        };
        if (type == IdentType::Type || type == IdentType::UserType) {
            if (type == IdentType::UserType || value < 12) {
                out.ident(value, "t");
            } else {
                out.raw(types[value % std::size(types)]);
            }
        } else if (type == IdentType::Function || type == IdentType::UserFunction) {
            if (type == IdentType::UserFunction || value < 12) {
                out.ident(value, "f");
            } else {
                out.raw(funcs[value % std::size(funcs)]);
            }
        } else {
            out.ident(value, "x");
        }
    };
}

void floatLiteral(TextOutput& out) {
    if (out.ctx.shouldUseVariable()) {
        out.raw(out.ctx.getRandomVariable());
    } else {
        out.raw("3.1416");
        std::string var = out.ctx.createVariable("f32");
        out.raw(" /* stored in " + var + " */");
    }
}

void floatHexLiteral(TextOutput& out) {
    if (out.ctx.shouldUseVariable()) {
        out.raw(out.ctx.getRandomVariable());
    } else {
        out.raw("0x1.Fp4");
        std::string var = out.ctx.createVariable("f32");
        out.raw(" /* stored in " + var + " */");
    }
}

void decimalLiteral(TextOutput& out) {
    if (out.ctx.shouldUseVariable()) {
        out.raw(out.ctx.getRandomVariable());
    } else {
        out.raw(std::to_string(out.ctx.gen.GetUInt32(1000)));
        std::string var = out.ctx.createVariable("i32");
        out.raw(" /* stored in " + var + " */");
    }
}

void hexLiteral(TextOutput& out) {
    if (out.ctx.shouldUseVariable()) {
        out.raw(out.ctx.getRandomVariable());
    } else {
        out.raw("0x" + std::to_string(out.ctx.gen.GetUInt32(0xFFFF)));
        std::string var = out.ctx.createVariable("i32");
        out.raw(" /* stored in " + var + " */");
    }
}

struct MutationStat {
    int alternatives = 0;
    int repeats[3]{0, 0, 0};
    int optionals[2]{0, 0};
    int terminals = 0;
    int transferLocations = 0; 
};

template <typename ByteIn, typename ByteOut>
void mutate(ByteIn& in,
            ByteOut& out,
            Mutation mutation,
            int& index,
            NodeId id,
            RandomGenerator& gen,
            Context& ctx,
            int depth = 0);

template <typename ByteIn, typename ByteOut>
void mutateOne(ByteIn& in,
               ByteOut& out,
               Mutation mutation,
               int& index,
               const Subnode& subnode,
               RandomGenerator& gen,
               Context& ctx,
               int depth = 0);

template <typename ByteIn, typename ByteOut>
void mutateAlt(ByteIn& in,
               ByteOut& out,
               Mutation mutation,
               int& index,
               const std::vector<Subnode>& subnodes,
               RandomGenerator& gen,
               Context& ctx,
               int depth = 0);

template <typename ByteIn, typename ByteOut>
void mutateOne(ByteIn& in,
               ByteOut& out,
               Mutation mutation,
               int& index,
               const Subnode& subnode,
               RandomGenerator& gen,
               Context& ctx,
               int depth) {
    if (const GenerateFn0* fn = std::get_if<GenerateFn0>(&subnode.content)) {
    } else if (const GenerateFn* fn = std::get_if<GenerateFn>(&subnode.content)) {
        uint8_t val = in.byteTerm();
        if (mutation == Mutation::RandomTerminal) {
            if (index-- == 0) {
                val = gen.GetUInt32(256);
            }
        }
        out.push(val);
    } else if (const NodeId* nodeId = std::get_if<NodeId>(&subnode.content)) {
        mutate(in, out, mutation, index, *nodeId, gen, ctx, depth + 1);
    }
}

template <typename ByteIn, typename ByteOut>
void mutateAlt(ByteIn& in,
               ByteOut& out,
               Mutation mutation,
               int& index,
               const std::vector<Subnode>& subnodes,
               RandomGenerator& gen,
               Context& ctx,
               int depth) {
    for (const Subnode& subnode : subnodes) {
        int repetitions = 1;
        int newRepetitions = 1;
        if (subnode.mod == '*') {
            newRepetitions = repetitions = in.range(maxRepeats + 1, true);
            if (mutation == Mutation::IncRepeat && repetitions < maxRepeats) {
                if (index-- == 0) {
                    ++newRepetitions;
                }
            }
            if (mutation == Mutation::DecRepeat && repetitions > 0) {
                if (index-- == 0) {
                    --newRepetitions;
                }
            }
            out.push(newRepetitions);
        } else if (subnode.mod == '?') {
            newRepetitions = repetitions = in.range(2, true);
            if (repetitions == 0 && mutation >= Mutation::AddOptional) {
                if (index-- == 0) {
                    newRepetitions = 1;
                }
            }
            if (repetitions == 1 && mutation >= Mutation::RemoveOptional) {
                if (index-- == 0) {
                    newRepetitions = 0;
                }
            }
            out.push(newRepetitions);
        }

        for (int i = 0; i < std::min(repetitions, newRepetitions); ++i) {
            mutateOne(in, out, mutation, index, subnode, gen, ctx, depth);
        }

        if (newRepetitions > repetitions) {
            ByteInputRnd rndIn{gen, ctx};
            for (int i = 0; i < newRepetitions - repetitions; ++i) {
                mutateOne(rndIn, out, mutation, index, subnode, gen, ctx, depth);
            }
        } else if (newRepetitions < repetitions) {
            ByteOutputNull nullOut;
            for (int i = 0; i < repetitions - newRepetitions; ++i) {
                mutateOne(in, nullOut, mutation, index, subnode, gen, ctx, depth);
            }
        }
    }
}

template <typename ByteIn, typename ByteOut>
void captureSubtree(ByteIn& in, ByteOut& out, NodeId id, Context& ctx, int depth = 0) {
    if (depth > maxDepth) {
        return;
    }
    const Node& node = nodes()[static_cast<int>(id)];
    uint8_t alternative = in.range(node.size(), false);
    out.push(alternative);
    
    auto& alt = node[alternative];
    for (const Subnode& subnode : alt) {
        int repetitions = 1;
        if (subnode.mod == '*') {
            repetitions = in.range(maxRepeats + 1, true);
            out.push(repetitions);
        } else if (subnode.mod == '?') {
            repetitions = in.range(2, true);
            out.push(repetitions);
        }
        
        for (int i = 0; i < repetitions; ++i) {
            if (const GenerateFn0* fn = std::get_if<GenerateFn0>(&subnode.content)) {

            } else if (const GenerateFn* fn = std::get_if<GenerateFn>(&subnode.content)) {
                uint8_t val = in.byteTerm();
                out.push(val);
            } else if (const NodeId* nodeId = std::get_if<NodeId>(&subnode.content)) {
                captureSubtree(in, out, *nodeId, ctx, depth + 1);
            }
        }
    }
}

template <typename ByteIn, typename ByteOut>
void applySubtree(ByteIn& in, ByteOut& out, NodeId id, Context& ctx, int depth = 0) {
    if (depth > maxDepth) {
        return;
    }
    const Node& node = nodes()[static_cast<int>(id)];
    uint8_t alternative = in.byte();
    if (alternative >= node.size()) {
        alternative = 0;  
    }
    out.push(alternative);
    
    auto& alt = node[alternative];
    for (const Subnode& subnode : alt) {
        int repetitions = 1;
        if (subnode.mod == '*') {
            repetitions = in.byte();
            if (repetitions > maxRepeats) {
                repetitions = maxRepeats;  
            }
            out.push(repetitions);
        } else if (subnode.mod == '?') {
            repetitions = in.byte();
            if (repetitions > 1) {
                repetitions = 1;  
            }
            out.push(repetitions);
        }
        
        for (int i = 0; i < repetitions; ++i) {
            if (const GenerateFn0* fn = std::get_if<GenerateFn0>(&subnode.content)) {

            } else if (const GenerateFn* fn = std::get_if<GenerateFn>(&subnode.content)) {
                uint8_t val = in.byte();
                out.push(val);
            } else if (const NodeId* nodeId = std::get_if<NodeId>(&subnode.content)) {
                applySubtree(in, out, *nodeId, ctx, depth + 1);
            }
        }
    }
}


bool areNodesCompatible(NodeId source, NodeId target) {


    if (source == target) return true;
    

    static const std::unordered_map<NodeId, std::vector<NodeId>> compatibilityGroups = {

        {NodeId::expression, {NodeId::unary_expression, NodeId::primary_expression}},
        {NodeId::unary_expression, {NodeId::expression, NodeId::primary_expression}},
        {NodeId::primary_expression, {NodeId::expression, NodeId::unary_expression}},
        

        {NodeId::statement, {NodeId::variable_or_value_statement, NodeId::compound_statement}},
        {NodeId::variable_or_value_statement, {NodeId::statement}},
        {NodeId::compound_statement, {NodeId::statement}},
        

        {NodeId::int_literal, {NodeId::float_literal, NodeId::literal}},
        {NodeId::float_literal, {NodeId::int_literal, NodeId::literal}},
        {NodeId::literal, {NodeId::int_literal, NodeId::float_literal}}
    };
    
    auto it = compatibilityGroups.find(source);
    if (it != compatibilityGroups.end()) {
        return std::find(it->second.begin(), it->second.end(), target) != it->second.end();
    }
    
    return false;
}

template <typename ByteIn, typename ByteOut>
void mutate(ByteIn& in,
            ByteOut& out,
            Mutation mutation,
            int& index,
            NodeId id,
            RandomGenerator& gen,
            Context& ctx,
            int depth) {
    if (depth > maxDepth) {
        return;
    }
    

    if (mutation == Mutation::SubtreeTransfer && index == 0 && !ctx.storedSubtree.empty()) {


        NodeId storedSubtreeNodeId = static_cast<NodeId>(ctx.storedSubtree[0]);
        if (areNodesCompatible(storedSubtreeNodeId, id)) {
            

            ByteInput storedIn{ctx.storedSubtree.data() + 1, ctx.storedSubtree.size() - 1, 0, true, ctx};
            applySubtree(storedIn, out, id, ctx, depth);
            return;
        } else {
            printf("[SUBTREE TRANSFER] Incompatible types: source %d, target %d\n",
                   static_cast<int>(storedSubtreeNodeId), static_cast<int>(id));
        }

    }
    

    if (mutation == Mutation::SubtreeTransfer && index == -1) {

        ByteOutput captureOut;
        captureOut.push(static_cast<uint8_t>(id)); 
        captureSubtree(in, captureOut, id, ctx, depth);
        ctx.storedSubtree = std::move(captureOut.out);


        index = -2; 

    }
    
    const Node& node = nodes()[static_cast<int>(id)];
    uint8_t alternative = 0;
    if (node.size() > 1) {
        alternative = in.range(node.size(), false);
        if (mutation >= Mutation::NextAlternative && mutation <= Mutation::RandomAlternative) {
            if (index-- == 0) {
                ByteOutputNull nullOut;
                mutateAlt(in, nullOut, mutation, index, node[alternative], gen, ctx, depth);
                switch (mutation) {
                    case Mutation::NextAlternative:
                        alternative = (alternative + 1) % node.size();
                        break;
                    case Mutation::PrevAlternative:
                        alternative = (alternative + node.size() - 1) % node.size();
                        break;
                    case Mutation::RandomAlternative: {
                        uint8_t newAlternative = gen.GetUInt32(node.size() - 1);
                        alternative = newAlternative >= alternative ? newAlternative + 1 : newAlternative;
                        break;
                    }
                    default:
                        break;
                }
                assert(alternative < node.size());
                out.push(alternative);
                ByteInputRnd rndIn{gen, ctx};
                mutateAlt(rndIn, out, mutation, index, node[alternative], gen, ctx, depth);
                return;
            }
        }
        out.push(alternative);
    }
    assert(alternative < node.size());
    mutateAlt(in, out, mutation, index, node[alternative], gen, ctx, depth);
}

void count(MutationStat& stat, ByteInput& in, NodeId id, int depth = 0) {
    if (depth > maxDepth) {
        return;
    }
    


    if (depth > 0 && id != NodeId::translation_unit) {
        ++stat.transferLocations;
    }
    
    const Node& node = nodes()[static_cast<int>(id)];
    if (node.size() > 1) {
        ++stat.alternatives;
    }
    uint8_t alternative = in.range(node.size(), false);
    auto& alt = node[alternative];
    for (const Subnode& subnode : alt) {
        int repetitions = 1;
        if (subnode.mod == '*') {
            repetitions = in.range(maxRepeats + 1, true);
            if (repetitions == 0) {
                ++stat.repeats[0];
            } else if (repetitions == maxRepeats) {
                ++stat.repeats[2];
            } else {
                ++stat.repeats[1];
            }
        } else if (subnode.mod == '?') {
            repetitions = in.range(2, true);
            ++stat.optionals[repetitions];
        }
        for (int i = 0; i < repetitions; ++i) {
            if (std::get_if<GenerateFn0>(&subnode.content)) {

            } else if (std::get_if<GenerateFn>(&subnode.content)) {
                in.byteTerm();
                ++stat.terminals;
            } else if (const NodeId* nodeId = std::get_if<NodeId>(&subnode.content)) {
                count(stat, in, *nodeId, depth + 1);
            }
        }
    }
}

void generate(ByteInput& in, TextOutput& out, NodeId id, int depth = 0) {
    if (depth > maxDepth) {
        return;
    }
    const Node& node = nodes()[static_cast<int>(id)];
    uint8_t alternative = in.range(node.size(), false);  // alternative
    auto& alt = node[alternative];
    for (const Subnode& subnode : alt) {
        int repetitions = 1;
        if (subnode.mod == '*') {
            repetitions = in.range(maxRepeats + 1, true);  // repeat
        } else if (subnode.mod == '?') {
            repetitions = in.range(2, true);  // optional
        }
        for (int i = 0; i < repetitions; ++i) {
            if (const GenerateFn0* fn = std::get_if<GenerateFn0>(&subnode.content)) {
                (*fn)(out);
            } else if (const GenerateFn* fn = std::get_if<GenerateFn>(&subnode.content)) {
                (*fn)(in.byteTerm(), out);
            } else if (const NodeId* nodeId = std::get_if<NodeId>(&subnode.content)) {
                generate(in, out, *nodeId, depth + 1);
            }
        }
    }
}

}  

std::vector<uint8_t> WGSLMutate(Mutation mutation,
                               const uint8_t* data,
                               size_t size,
                               RandomGenerator& gen) {
    Context ctx(gen);  
    ByteInput in{data, size, 0, true, ctx};  
    ByteOutput out{};
    MutationStat stat{};
    int index = -1;
    

    if (mutation == Mutation::LibFuzzerMutate && size > 0) {

        out.out.assign(data, data + size);
        

        int numMutations = 1 + gen.GetUInt32(std::min(3u, static_cast<uint32_t>(size / 10 + 1)));
        
        for (int i = 0; i < numMutations; i++) {

            int strategy = gen.GetUInt32(5);
            
            switch (strategy) {
                case 0: { // Bit flip
                    size_t pos = gen.GetUInt32(size);
                    uint8_t mask = 1 << gen.GetUInt32(8);
                    out.out[pos] ^= mask;
                    break;
                }
                case 1: { 
                    size_t pos = gen.GetUInt32(size);
                    out.out[pos] ^= 0xFF;
                    break;
                }
                case 2: { 
                    size_t pos = gen.GetUInt32(size);
                    out.out[pos] = gen.GetUInt32(256);
                    break;
                }
                case 3: {
                    if (size > 1) {
                        size_t pos = gen.GetUInt32(size - 1);
                        std::swap(out.out[pos], out.out[pos + 1]);
                    }
                    break;
                }
                case 4: { 
                    if (out.out.size() < 65536) { 
                        size_t pos = gen.GetUInt32(out.out.size() + 1);
                        out.out.insert(out.out.begin() + pos, gen.GetUInt32(256));
                    }
                    break;
                }
            }
        }
        
        return std::move(out.out);
    }

    count(stat, in, NodeId::translation_unit);
    
    while (index < 0) {
        index = -1;
        switch (mutation) {
            case Mutation::AddOptional:
                if (stat.optionals[0] > 0) {
                    index = gen.GetUInt32(stat.optionals[0]);
                }
                break;
            case Mutation::RemoveOptional:
                if (stat.optionals[1] > 0) {
                    index = gen.GetUInt32(stat.optionals[1]);
                }
                break;
            case Mutation::IncRepeat:
                if (stat.repeats[0] + stat.repeats[1] > 0) {
                    index = gen.GetUInt32(stat.repeats[0] + stat.repeats[1]);
                }
                break;
            case Mutation::DecRepeat:
                if (stat.repeats[1] + stat.repeats[2] > 0) {
                    index = gen.GetUInt32(stat.repeats[1] + stat.repeats[2]);
                }
                break;
            case Mutation::NextAlternative:
            case Mutation::PrevAlternative:
            case Mutation::RandomAlternative:
                if (stat.alternatives > 0) {
                    index = gen.GetUInt32(stat.alternatives);
                }
                break;
            case Mutation::RandomTerminal:
                if (stat.terminals > 0) {
                    index = gen.GetUInt32(stat.terminals);
                }
                break;
            case Mutation::SubtreeTransfer:


                if (stat.transferLocations > 1) { 
                    index = -1; 
                }
                break;
            case Mutation::LibFuzzerMutate:

                index = 0;
                break;
        }
        if (index == -1) {
            mutation = static_cast<Mutation>((static_cast<unsigned>(mutation) + 1) %
                                           (static_cast<unsigned>(Mutation::Last) + 1));
        }
    }
    


    if (mutation == Mutation::SubtreeTransfer) {

        in.reset();
        index = -1; 
        mutate(in, out, mutation, index, NodeId::translation_unit, gen, ctx, 0);
        

        if (index == -2 && !ctx.storedSubtree.empty()) {

            in.reset();
            out.out.clear();
            index = gen.GetUInt32(stat.transferLocations); 
            mutate(in, out, mutation, index, NodeId::translation_unit, gen, ctx, 0);
        } else {

            in.reset();
            out.out.clear();
            mutation = Mutation::RandomTerminal;
            index = gen.GetUInt32(stat.terminals);
            mutate(in, out, mutation, index, NodeId::translation_unit, gen, ctx, 0);
        }
    } else {

        in.reset();
        mutate(in, out, mutation, index, NodeId::translation_unit, gen, ctx, 0);
    }
    
    return std::move(out.out);
}

std::string WGSLSource(const uint8_t* data, size_t size) {
    RandomGenerator gen{CRC32(data, size)};
    Context ctx(gen);
    ByteInput in{data, size, 0, false, ctx};
    TextOutput out(ctx);
    generate(in, out, NodeId::translation_unit);
    return std::move(out.buffer).str();
}

}  // namespace tint::fuzzers::structure_fuzzer


