#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

f16vec2 u = f16vec2(1.0hf);
uvec2 tint_v2f16_to_v2u32(f16vec2 value) {
  uvec2 v_1 = uvec2(value);
  bvec2 v_2 = greaterThanEqual(value, f16vec2(0.0hf));
  uint v_3 = ((v_2.x) ? (v_1.x) : (uvec2(0u).x));
  uvec2 v_4 = uvec2(v_3, ((v_2.y) ? (v_1.y) : (uvec2(0u).y)));
  bvec2 v_5 = lessThanEqual(value, f16vec2(65504.0hf));
  uint v_6 = ((v_5.x) ? (v_4.x) : (uvec2(4294967295u).x));
  return uvec2(v_6, ((v_5.y) ? (v_4.y) : (uvec2(4294967295u).y)));
}
void f() {
  uvec2 v = tint_v2f16_to_v2u32(u);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
