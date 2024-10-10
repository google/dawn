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
  gl_Position = vertex_main_inner();
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  gl_PointSize = 1.0f;
}
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
#version 310 es


struct RenderParams {
  mat4 modelViewProjectionMatrix;
  vec3 right;
  uint tint_pad_0;
  vec3 up;
  uint tint_pad_1;
};

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

layout(binding = 5, std140)
uniform render_params_block_1_ubo {
  RenderParams inner;
} v;
layout(location = 0) in vec3 vs_main_loc0_Input;
layout(location = 1) in vec4 vs_main_loc1_Input;
layout(location = 2) in vec2 vs_main_loc2_Input;
layout(location = 0) out vec4 vs_main_loc0_Output;
layout(location = 1) out vec2 vs_main_loc1_Output;
VertexOutput vs_main_inner(VertexInput tint_symbol) {
  vec3 quad_pos = (mat2x3(v.inner.right, v.inner.up) * tint_symbol.quad_pos);
  vec3 position = (tint_symbol.position - (quad_pos + 0.00999999977648258209f));
  VertexOutput tint_symbol_1 = VertexOutput(vec4(0.0f), vec4(0.0f), vec2(0.0f));
  mat4 v_1 = v.inner.modelViewProjectionMatrix;
  tint_symbol_1.position = (v_1 * vec4(position, 1.0f));
  tint_symbol_1.color = tint_symbol.color;
  tint_symbol_1.quad_pos = tint_symbol.quad_pos;
  return tint_symbol_1;
}
void main() {
  VertexOutput v_2 = vs_main_inner(VertexInput(vs_main_loc0_Input, vs_main_loc1_Input, vs_main_loc2_Input));
  gl_Position = v_2.position;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  vs_main_loc0_Output = v_2.color;
  vs_main_loc1_Output = v_2.quad_pos;
  gl_PointSize = 1.0f;
}
#version 310 es


struct SimulationParams {
  float deltaTime;
  uint tint_pad_0;
  uint tint_pad_1;
  uint tint_pad_2;
  vec4 seed;
};

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
  SimulationParams inner;
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
  vec2 v_1 = v.inner.seed.xy;
  vec2 v_2 = (v_1 * vec2(GlobalInvocationID.xy));
  rand_seed = (v_2 * v.inner.seed.zw);
  uint idx = GlobalInvocationID[0u];
  Particle particle = data.particles[idx];
  Particle v_3 = particle;
  tint_store_and_preserve_padding(uint[1](idx), v_3);
}
layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;
void main() {
  simulate_inner(gl_GlobalInvocationID);
}
#version 310 es


struct UBO {
  uint width;
};

layout(binding = 3, std140)
uniform ubo_block_1_ubo {
  UBO inner;
} v;
layout(binding = 4, std430)
buffer Buffer_1_ssbo {
  float weights[];
} buf_in;
layout(binding = 5, std430)
buffer Buffer_2_ssbo {
  float weights[];
} buf_out;
layout(binding = 7, rgba8) uniform highp writeonly image2D tex_out;
float tint_float_modulo(float x, float y) {
  return (x - (y * trunc((x / y))));
}
void export_level_inner(uvec3 coord) {
  if (all(lessThan(coord.xy, uvec2(uvec2(imageSize(tex_out)))))) {
    uint dst_offset = (coord[0u] << ((coord[1u] * v.inner.width) & 31u));
    uint src_offset = ((coord[0u] - 2u) + ((coord[1u] >> (2u & 31u)) * v.inner.width));
    float a = buf_in.weights[(src_offset << (0u & 31u))];
    float b = buf_in.weights[(src_offset + 1u)];
    uint v_1 = ((src_offset + 1u) + v.inner.width);
    float c = buf_in.weights[v_1];
    uint v_2 = ((src_offset + 1u) + v.inner.width);
    float d = buf_in.weights[v_2];
    float sum = dot(vec4(a, b, c, d), vec4(1.0f));
    buf_out.weights[dst_offset] = tint_float_modulo(sum, 4.0f);
    vec4 v_3 = vec4(a, (a * b), ((a / b) + c), sum);
    vec4 probabilities = (v_3 + max(sum, 0.0f));
    imageStore(tex_out, ivec2(coord.xy), probabilities);
  }
}
layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;
void main() {
  export_level_inner(gl_GlobalInvocationID);
}
