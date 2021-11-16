#version 310 es
precision mediump float;

struct MyStruct {
  float f1;
};

const int v1 = 1;
const uint v2 = 1u;
const float v3 = 1.0f;
const ivec3 v4 = ivec3(1, 1, 1);
const uvec3 v5 = uvec3(1u, 1u, 1u);
const vec3 v6 = vec3(1.0f, 1.0f, 1.0f);
const mat3 v7 = mat3(vec3(1.0f, 1.0f, 1.0f), vec3(1.0f, 1.0f, 1.0f), vec3(1.0f, 1.0f, 1.0f));
const MyStruct v8 = MyStruct(0.0f);
const float v9[10] = float[10](0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);

struct tint_symbol_1 {
  vec4 value;
};

vec4 tint_symbol_inner() {
  return vec4(0.0f, 0.0f, 0.0f, 0.0f);
}

tint_symbol_1 tint_symbol() {
  vec4 inner_result = tint_symbol_inner();
  tint_symbol_1 wrapper_result = tint_symbol_1(vec4(0.0f, 0.0f, 0.0f, 0.0f));
  wrapper_result.value = inner_result;
  return wrapper_result;
}
out vec4 value;
void main() {
  tint_symbol_1 outputs;
  outputs = tint_symbol();
  value = outputs.value;
}


