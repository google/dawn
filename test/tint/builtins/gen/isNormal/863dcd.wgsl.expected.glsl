builtins/gen/isNormal/863dcd.wgsl:28:25 warning: use of deprecated builtin
  var res: vec4<bool> = isNormal(vec4<f32>());
                        ^^^^^^^^

#version 310 es

bvec4 tint_isNormal(vec4 param_0) {
  uvec4 exponent = floatBitsToUint(param_0) & 0x7f80000u;
  uvec4 clamped = clamp(exponent, 0x0080000u, 0x7f00000u);
  return equal(clamped, exponent);
}


void isNormal_863dcd() {
  bvec4 res = tint_isNormal(vec4(0.0f, 0.0f, 0.0f, 0.0f));
}

vec4 vertex_main() {
  isNormal_863dcd();
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

bvec4 tint_isNormal(vec4 param_0) {
  uvec4 exponent = floatBitsToUint(param_0) & 0x7f80000u;
  uvec4 clamped = clamp(exponent, 0x0080000u, 0x7f00000u);
  return equal(clamped, exponent);
}


void isNormal_863dcd() {
  bvec4 res = tint_isNormal(vec4(0.0f, 0.0f, 0.0f, 0.0f));
}

void fragment_main() {
  isNormal_863dcd();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

bvec4 tint_isNormal(vec4 param_0) {
  uvec4 exponent = floatBitsToUint(param_0) & 0x7f80000u;
  uvec4 clamped = clamp(exponent, 0x0080000u, 0x7f00000u);
  return equal(clamped, exponent);
}


void isNormal_863dcd() {
  bvec4 res = tint_isNormal(vec4(0.0f, 0.0f, 0.0f, 0.0f));
}

void compute_main() {
  isNormal_863dcd();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
