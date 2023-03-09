#version 310 es
precision highp float;

layout(location = 0) out vec4 value;
vec4 frag_main() {
  float b = 0.0f;
  vec3 v = vec3(b);
  return vec4(v, 1.0f);
}

void main() {
  vec4 inner_result = frag_main();
  value = inner_result;
  return;
}
