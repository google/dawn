#version 310 es

vec4 tint_degrees(vec4 param_0) {
  return param_0 * 57.295779513082322865f;
}

vec3 tint_degrees_1(vec3 param_0) {
  return param_0 * 57.295779513082322865f;
}

vec2 tint_degrees_2(vec2 param_0) {
  return param_0 * 57.295779513082322865f;
}

float tint_degrees_3(float param_0) {
  return param_0 * 57.295779513082322865f;
}


void tint_symbol() {
  tint_degrees(vec4(0.0f));
  tint_degrees(vec4(1.0f));
  tint_degrees(vec4(1.0f, 2.0f, 3.0f, 4.0f));
  tint_degrees_1(vec3(0.0f));
  tint_degrees_1(vec3(1.0f));
  tint_degrees_1(vec3(1.0f, 2.0f, 3.0f));
  tint_degrees_2(vec2(0.0f));
  tint_degrees_2(vec2(1.0f));
  tint_degrees_2(vec2(1.0f, 2.0f));
  tint_degrees_3(1.0f);
  tint_degrees_3(2.0f);
  tint_degrees_3(3.0f);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
