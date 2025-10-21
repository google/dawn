//
// vertex_main
//
#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

void asinh_468a48() {
  float16_t arg_0 = 0.0hf;
  float16_t res = asinh(arg_0);
}
vec4 vertex_main_inner() {
  asinh_468a48();
  return vec4(0.0f);
}
void main() {
  vec4 v = vertex_main_inner();
  gl_Position = vec4(v.x, -(v.y), ((2.0f * v.z) - v.w), v.w);
  gl_PointSize = 1.0f;
}
//
// fragment_main
//
#version 310 es
#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;

void asinh_468a48() {
  float16_t arg_0 = 0.0hf;
  float16_t res = asinh(arg_0);
}
void main() {
  asinh_468a48();
}
//
// rgba32uintin
//
#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

void asinh_468a48() {
  float16_t arg_0 = 0.0hf;
  float16_t res = asinh(arg_0);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  asinh_468a48();
}
//
// vs_main
//
#version 310 es


struct VertexOutput {
  vec4 position;
  vec4 color;
  vec2 quad_pos;
};

struct VertexInput {
  vec3 position;
  vec4 color;
  vec2 quad_pos;
};

layout(binding = 0, std140)
uniform v_render_params_block_ubo {
  uvec4 inner[6];
} v;
layout(location = 0) in vec3 vs_main_loc0_Input;
layout(location = 1) in vec4 vs_main_loc1_Input;
layout(location = 2) in vec2 vs_main_loc2_Input;
layout(location = 0) out vec4 tint_interstage_location0;
layout(location = 1) out vec2 tint_interstage_location1;
mat4 v_1(uint start_byte_offset) {
  return mat4(uintBitsToFloat(v.inner[(start_byte_offset / 16u)]), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)]), uintBitsToFloat(v.inner[((32u + start_byte_offset) / 16u)]), uintBitsToFloat(v.inner[((48u + start_byte_offset) / 16u)]));
}
VertexOutput vs_main_inner(VertexInput v_2) {
  vec3 quad_pos = (mat2x3(uintBitsToFloat(v.inner[4u].xyz), uintBitsToFloat(v.inner[5u].xyz)) * v_2.quad_pos);
  vec3 position = (v_2.position - (quad_pos + 0.00999999977648258209f));
  VertexOutput v_3 = VertexOutput(vec4(0.0f), vec4(0.0f), vec2(0.0f));
  mat4 v_4 = v_1(0u);
  v_3.position = (v_4 * vec4(position, 1.0f));
  v_3.color = v_2.color;
  v_3.quad_pos = v_2.quad_pos;
  return v_3;
}
void main() {
  VertexOutput v_5 = vs_main_inner(VertexInput(vs_main_loc0_Input, vs_main_loc1_Input, vs_main_loc2_Input));
  gl_Position = vec4(v_5.position.x, -(v_5.position.y), ((2.0f * v_5.position.z) - v_5.position.w), v_5.position.w);
  tint_interstage_location0 = v_5.color;
  tint_interstage_location1 = v_5.quad_pos;
  gl_PointSize = 1.0f;
}
//
// simulate
//
#version 310 es


struct Particle {
  vec3 position;
  float lifetime;
  vec4 color;
  vec2 velocity;
  uint tint_pad_0;
  uint tint_pad_1;
};

vec2 rand_seed = vec2(0.0f);
layout(binding = 0, std140)
uniform sim_params_block_1_ubo {
  uvec4 inner[2];
} v;
layout(binding = 1, std430)
buffer Particles_1_ssbo {
  Particle particles[];
} data;
void tint_store_and_preserve_padding(uint target_indices[1], Particle value_param) {
  data.particles[target_indices[0u]].position = value_param.position;
  data.particles[target_indices[0u]].lifetime = value_param.lifetime;
  data.particles[target_indices[0u]].color = value_param.color;
  data.particles[target_indices[0u]].velocity = value_param.velocity;
}
void simulate_inner(uvec3 GlobalInvocationID) {
  vec2 v_1 = uintBitsToFloat(v.inner[1u]).xy;
  vec2 v_2 = (v_1 * vec2(GlobalInvocationID.xy));
  rand_seed = (v_2 * uintBitsToFloat(v.inner[1u]).zw);
  uint idx = GlobalInvocationID.x;
  uint v_3 = min(idx, (uint(data.particles.length()) - 1u));
  Particle particle = data.particles[v_3];
  uint v_4 = min(idx, (uint(data.particles.length()) - 1u));
  Particle v_5 = particle;
  tint_store_and_preserve_padding(uint[1](v_4), v_5);
}
layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;
void main() {
  simulate_inner(gl_GlobalInvocationID);
}
//
// export_level
//
#version 310 es

layout(binding = 0, std140)
uniform ubo_block_1_ubo {
  uvec4 inner[1];
} v;
layout(binding = 1, std430)
buffer Buffer_1_ssbo {
  float weights[];
} buf_in;
layout(binding = 2, std430)
buffer Buffer_2_ssbo {
  float weights[];
} buf_out;
layout(binding = 3, rgba8) uniform highp writeonly image2D tex_out;
float tint_float_modulo(float x, float y) {
  return (x - (y * trunc((x / y))));
}
void export_level_inner(uvec3 coord) {
  if (all(lessThan(coord.xy, uvec2(uvec2(imageSize(tex_out)))))) {
    uvec4 v_1 = v.inner[0u];
    uint dst_offset = (coord.x << ((coord.y * v_1.x) & 31u));
    uvec4 v_2 = v.inner[0u];
    uint src_offset = ((coord.x - 2u) + ((coord.y >> (2u & 31u)) * v_2.x));
    uint v_3 = min((src_offset << (0u & 31u)), (uint(buf_in.weights.length()) - 1u));
    float a = buf_in.weights[v_3];
    uint v_4 = min((src_offset + 1u), (uint(buf_in.weights.length()) - 1u));
    float b = buf_in.weights[v_4];
    uvec4 v_5 = v.inner[0u];
    uint v_6 = min(((src_offset + 1u) + v_5.x), (uint(buf_in.weights.length()) - 1u));
    float c = buf_in.weights[v_6];
    uvec4 v_7 = v.inner[0u];
    uint v_8 = min(((src_offset + 1u) + v_7.x), (uint(buf_in.weights.length()) - 1u));
    float d = buf_in.weights[v_8];
    float sum = dot(vec4(a, b, c, d), vec4(1.0f));
    uint v_9 = min(dst_offset, (uint(buf_out.weights.length()) - 1u));
    buf_out.weights[v_9] = tint_float_modulo(sum, 4.0f);
    vec4 probabilities = (vec4(a, (a * b), ((a / b) + c), sum) + max(sum, 0.0f));
    imageStore(tex_out, ivec2(coord.xy), probabilities);
  }
}
layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;
void main() {
  export_level_inner(gl_GlobalInvocationID);
}
