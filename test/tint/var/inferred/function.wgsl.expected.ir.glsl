SKIP: FAILED

#version 310 es

struct MyStruct {
  float f1;
};
precision highp float;
precision highp int;


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
  return MyStruct(0.0f);
}
float[10] ret_MyArray() {
  return float[10](0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
}
void var_decls() {
  int v1 = 1;
  uint v2 = 1u;
  float v3 = 1.0f;
  ivec3 v4 = ivec3(1);
  uvec3 v5 = uvec3(1u);
  vec3 v6 = vec3(1.0f);
  mat3 v7 = mat3(v6, v6, v6);
  MyStruct v8 = MyStruct(1.0f);
  float v9[10] = float[10](0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  int v10 = ret_i32();
  uint v11 = ret_u32();
  float v12 = ret_f32();
  MyStruct v13 = ret_MyStruct();
  MyStruct v14 = ret_MyStruct();
  float v15[10] = ret_MyArray();
}
vec4 main() {
  return vec4(0.0f);
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
