#version 310 es
precision highp float;

layout(location = 0) out vec4 value;
struct MyStruct {
  float f1;
};

vec4 tint_symbol() {
  return vec4(0.0f);
}

void main() {
  vec4 inner_result = tint_symbol();
  value = inner_result;
  return;
}
