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
  MyStruct tint_symbol_1 = (MyStruct)0;
  return tint_symbol_1;
}

typedef float ret_MyArray_ret[10];
ret_MyArray_ret ret_MyArray() {
  float tint_symbol_2[10] = (float[10])0;
  return tint_symbol_2;
}

void let_decls() {
  int v1 = 1;
  uint v2 = 1u;
  float v3 = 1.0f;
  int3 v4 = (1).xxx;
  uint3 v5 = (1u).xxx;
  float3 v6 = (1.0f).xxx;
  float3x3 v7 = float3x3(v6, v6, v6);
  MyStruct v8 = {1.0f};
  float v9[10] = (float[10])0;
  int v10 = ret_i32();
  uint v11 = ret_u32();
  float v12 = ret_f32();
  MyStruct v13 = ret_MyStruct();
  MyStruct v14 = ret_MyStruct();
  float v15[10] = ret_MyArray();
}

struct tint_symbol {
  float4 value : SV_Target0;
};

float4 main_inner() {
  return (0.0f).xxxx;
}

tint_symbol main() {
  float4 inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}
