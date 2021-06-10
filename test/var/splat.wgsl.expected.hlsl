struct tint_symbol {
  float4 value : SV_Target0;
};

void from_immediate_bool() {
  bool2 v2 = bool2((true).xx);
  bool3 v3 = bool3((true).xxx);
  bool4 v4 = bool4((true).xxxx);
}

void from_immediate_f32() {
  float2 v2 = float2((1.0f).xx);
  float3 v3 = float3((1.0f).xxx);
  float4 v4 = float4((1.0f).xxxx);
}

void from_immediate_i32() {
  int2 v2 = int2((1).xx);
  int3 v3 = int3((1).xxx);
  int4 v4 = int4((1).xxxx);
}

void from_immediate_u32() {
  uint2 v2 = uint2((1u).xx);
  uint3 v3 = uint3((1u).xxx);
  uint4 v4 = uint4((1u).xxxx);
}

void from_expression_bool() {
  bool2 v2 = bool2((true).xx);
  bool3 v3 = bool3((true).xxx);
  bool4 v4 = bool4((true).xxxx);
}

void from_expression_f32() {
  float2 v2 = float2(((1.0f + 2.0f)).xx);
  float3 v3 = float3(((1.0f + 2.0f)).xxx);
  float4 v4 = float4(((1.0f + 2.0f)).xxxx);
}

void from_expression_i32() {
  int2 v2 = int2(((1 + 2)).xx);
  int3 v3 = int3(((1 + 2)).xxx);
  int4 v4 = int4(((1 + 2)).xxxx);
}

void from_expression_u32() {
  uint2 v2 = uint2(((1u + 2u)).xx);
  uint3 v3 = uint3(((1u + 2u)).xxx);
  uint4 v4 = uint4(((1u + 2u)).xxxx);
}

bool get_bool() {
  return true;
}

float get_f32() {
  return 1.0f;
}

int get_i32() {
  return 1;
}

uint get_u32() {
  return 1u;
}

void from_call_bool() {
  bool2 v2 = bool2((get_bool()).xx);
  bool3 v3 = bool3((get_bool()).xxx);
  bool4 v4 = bool4((get_bool()).xxxx);
}

void from_call_f32() {
  float2 v2 = float2((get_f32()).xx);
  float3 v3 = float3((get_f32()).xxx);
  float4 v4 = float4((get_f32()).xxxx);
}

void from_call_i32() {
  int2 v2 = int2((get_i32()).xx);
  int3 v3 = int3((get_i32()).xxx);
  int4 v4 = int4((get_i32()).xxxx);
}

void from_call_u32() {
  uint2 v2 = uint2((get_u32()).xx);
  uint3 v3 = uint3((get_u32()).xxx);
  uint4 v4 = uint4((get_u32()).xxxx);
}

void with_swizzle() {
  float a = float2((1.0f).xx).y;
  float b = float3((1.0f).xxx).z;
  float c = float4((1.0f).xxxx).w;
}

tint_symbol main() {
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

