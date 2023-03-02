#version 310 es

vec4 tint_degrees(vec4 param_0) {
  return param_0 * 57.29577951308232286465f;
}

vec3 tint_degrees_1(vec3 param_0) {
  return param_0 * 57.29577951308232286465f;
}

vec2 tint_degrees_2(vec2 param_0) {
  return param_0 * 57.29577951308232286465f;
}

float tint_degrees_3(float param_0) {
  return param_0 * 57.29577951308232286465f;
}


void tint_symbol() {
  vec4 va = vec4(0.0f);
  vec4 a = tint_degrees(va);
  vec4 vb = vec4(1.0f);
  vec4 b = tint_degrees(vb);
  vec4 vc = vec4(1.0f, 2.0f, 3.0f, 4.0f);
  vec4 c = tint_degrees(vc);
  vec3 vd = vec3(0.0f);
  vec3 d = tint_degrees_1(vd);
  vec3 ve = vec3(1.0f);
  vec3 e = tint_degrees_1(ve);
  vec3 vf = vec3(1.0f, 2.0f, 3.0f);
  vec3 f = tint_degrees_1(vf);
  vec2 vg = vec2(0.0f);
  vec2 g = tint_degrees_2(vg);
  vec2 vh = vec2(1.0f);
  vec2 h = tint_degrees_2(vh);
  vec2 vi = vec2(1.0f, 2.0f);
  vec2 i = tint_degrees_2(vi);
  float vj = 1.0f;
  float j = tint_degrees_3(vj);
  float vk = 2.0f;
  float k = tint_degrees_3(vk);
  float vl = 3.0f;
  float l = tint_degrees_3(vl);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
