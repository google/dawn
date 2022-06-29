struct MyStruct {
  float f1;
};

int ret_i32() {
  return 1;
}

uint ret_u32() {
  return 1u;
}

float ret_f32() {
  return 1.0f;
}

MyStruct ret_MyStruct() {
  const MyStruct tint_symbol_1 = (MyStruct)0;
  return tint_symbol_1;
}

typedef float ret_MyArray_ret[10];
ret_MyArray_ret ret_MyArray() {
  const float tint_symbol_2[10] = (float[10])0;
  return tint_symbol_2;
}

void let_decls() {
  const int v1 = 1;
  const uint v2 = 1u;
  const float v3 = 1.0f;
  const int3 v4 = (1).xxx;
  const uint3 v5 = (1u).xxx;
  const float3 v6 = (1.0f).xxx;
  const float3x3 v7 = float3x3(v6, v6, v6);
  const MyStruct v8 = {1.0f};
  const float v9[10] = (float[10])0;
  const int v10 = ret_i32();
  const uint v11 = ret_u32();
  const float v12 = ret_f32();
  const MyStruct v13 = ret_MyStruct();
  const MyStruct v14 = ret_MyStruct();
  const float v15[10] = ret_MyArray();
}

struct tint_symbol {
  float4 value : SV_Target0;
};

float4 main_inner() {
  return (0.0f).xxxx;
}

tint_symbol main() {
  const float4 inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}
