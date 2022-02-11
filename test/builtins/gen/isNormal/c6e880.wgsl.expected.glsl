builtins/gen/isNormal/c6e880.wgsl:28:19 warning: use of deprecated builtin
  var res: bool = isNormal(1.0);
                  ^^^^^^^^

#version 310 es

bool tint_isNormal(float param_0) {
  uint exponent = floatBitsToUint(param_0) & 0x7f80000u;
  uint clamped = clamp(exponent, 0x0080000u, 0x7f00000u);
  return clamped == exponent;
}


void isNormal_c6e880() {
  bool res = tint_isNormal(1.0f);
}

vec4 vertex_main() {
  isNormal_c6e880();
  return vec4(0.0f, 0.0f, 0.0f, 0.0f);
}

void main() {
  vec4 inner_result = vertex_main();
  gl_Position = inner_result;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
#version 310 es
precision mediump float;

bool tint_isNormal(float param_0) {
  uint exponent = floatBitsToUint(param_0) & 0x7f80000u;
  uint clamped = clamp(exponent, 0x0080000u, 0x7f00000u);
  return clamped == exponent;
}


void isNormal_c6e880() {
  bool res = tint_isNormal(1.0f);
}

void fragment_main() {
  isNormal_c6e880();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

bool tint_isNormal(float param_0) {
  uint exponent = floatBitsToUint(param_0) & 0x7f80000u;
  uint clamped = clamp(exponent, 0x0080000u, 0x7f00000u);
  return clamped == exponent;
}


void isNormal_c6e880() {
  bool res = tint_isNormal(1.0f);
}

void compute_main() {
  isNormal_c6e880();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
