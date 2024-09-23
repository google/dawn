struct MyStruct {
  float f1;
};

struct main_outputs {
  float4 tint_symbol : SV_Target0;
};


int ret_i32() {
  return int(1);
}

uint ret_u32() {
  return 1u;
}

float ret_f32() {
  return 1.0f;
}

MyStruct ret_MyStruct() {
  MyStruct v = (MyStruct)0;
  return v;
}

typedef float ary_ret[10];
ary_ret ret_MyArray() {
  float v_1[10] = (float[10])0;
  return v_1;
}

void var_decls() {
  int v1 = int(1);
  uint v2 = 1u;
  float v3 = 1.0f;
  int3 v4 = (int(1)).xxx;
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

float4 main_inner() {
  return (0.0f).xxxx;
}

main_outputs main() {
  main_outputs v_2 = {main_inner()};
  return v_2;
}

