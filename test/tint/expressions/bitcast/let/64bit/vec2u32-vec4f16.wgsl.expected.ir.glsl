#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

f16vec4 tint_bitcast_to_f16(uvec2 src) {
  uvec2 v = uvec2(src);
  f16vec2 v_1 = unpackFloat2x16(v.x);
  return f16vec4(v_1, unpackFloat2x16(v.y));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uvec2 a = uvec2(1073757184u, 3288351232u);
  f16vec4 b = tint_bitcast_to_f16(a);
}
