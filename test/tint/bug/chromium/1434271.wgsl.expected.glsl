#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

void asinh_468a48() {
  float16_t arg_0 = 0.0hf;
  float16_t res = asinh(arg_0);
}

struct TestData {
  int dmat2atxa2[4];
};

struct RenderParams {
  mat4 modelViewProjectionMatrix;
  vec3 right;
  vec3 up;
};

struct VertexInput {
  vec3 position;
  vec4 color;
  vec2 quad_pos;
};

struct VertexOutput {
  vec4 position;
  vec4 color;
  vec2 quad_pos;
};

struct SimulationParams {
  float deltaTime;
  vec4 seed;
};

struct Particle {
  vec3 position;
  float lifetime;
  vec4 color;
  vec2 velocity;
};

struct UBO {
  uint width;
};

vec4 vertex_main() {
  asinh_468a48();
  return vec4(0.0f);
}

void main() {
  gl_PointSize = 1.0;
  vec4 inner_result = vertex_main();
  gl_Position = inner_result;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
#version 310 es
#extension GL_AMD_gpu_shader_half_float : require
precision highp float;
precision highp int;

void asinh_468a48() {
  float16_t arg_0 = 0.0hf;
  float16_t res = asinh(arg_0);
}

struct TestData {
  int dmat2atxa2[4];
};

struct RenderParams {
  mat4 modelViewProjectionMatrix;
  vec3 right;
  vec3 up;
};

struct VertexInput {
  vec3 position;
  vec4 color;
  vec2 quad_pos;
};

struct VertexOutput {
  vec4 position;
  vec4 color;
  vec2 quad_pos;
};

struct SimulationParams {
  float deltaTime;
  vec4 seed;
};

struct Particle {
  vec3 position;
  float lifetime;
  vec4 color;
  vec2 velocity;
};

struct UBO {
  uint width;
};

void fragment_main() {
  asinh_468a48();
}

void main() {
  fragment_main();
  return;
}
#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

void asinh_468a48() {
  float16_t arg_0 = 0.0hf;
  float16_t res = asinh(arg_0);
}

struct TestData {
  int dmat2atxa2[4];
};

struct RenderParams {
  mat4 modelViewProjectionMatrix;
  vec3 right;
  vec3 up;
};

struct VertexInput {
  vec3 position;
  vec4 color;
  vec2 quad_pos;
};

struct VertexOutput {
  vec4 position;
  vec4 color;
  vec2 quad_pos;
};

struct SimulationParams {
  float deltaTime;
  vec4 seed;
};

struct Particle {
  vec3 position;
  float lifetime;
  vec4 color;
  vec2 velocity;
};

struct UBO {
  uint width;
};

void rgba32uintin() {
  asinh_468a48();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  rgba32uintin();
  return;
}
#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

layout(location = 0) in vec3 position_1;
layout(location = 1) in vec4 color_1;
layout(location = 2) in vec2 quad_pos_1;
layout(location = 0) out vec4 color_2;
layout(location = 1) out vec2 quad_pos_2;
struct TestData {
  int dmat2atxa2[4];
};

struct RenderParams {
  mat4 modelViewProjectionMatrix;
  vec3 right;
  uint pad;
  vec3 up;
  uint pad_1;
};

layout(binding = 5, std140) uniform render_params_block_ubo {
  RenderParams inner;
} render_params;

struct VertexInput {
  vec3 position;
  vec4 color;
  vec2 quad_pos;
};

struct VertexOutput {
  vec4 position;
  vec4 color;
  vec2 quad_pos;
};

struct SimulationParams {
  float deltaTime;
  vec4 seed;
};

struct Particle {
  vec3 position;
  float lifetime;
  vec4 color;
  vec2 velocity;
};

struct UBO {
  uint width;
};

VertexOutput vs_main(VertexInput tint_symbol) {
  vec3 quad_pos = (mat2x3(render_params.inner.right, render_params.inner.up) * tint_symbol.quad_pos);
  vec3 position = (tint_symbol.position - (quad_pos + 0.00999999977648258209f));
  VertexOutput tint_symbol_1 = VertexOutput(vec4(0.0f, 0.0f, 0.0f, 0.0f), vec4(0.0f, 0.0f, 0.0f, 0.0f), vec2(0.0f, 0.0f));
  tint_symbol_1.position = (render_params.inner.modelViewProjectionMatrix * vec4(position, 1.0f));
  tint_symbol_1.color = tint_symbol.color;
  tint_symbol_1.quad_pos = tint_symbol.quad_pos;
  return tint_symbol_1;
}

void main() {
  gl_PointSize = 1.0;
  VertexInput tint_symbol_3 = VertexInput(position_1, color_1, quad_pos_1);
  VertexOutput inner_result = vs_main(tint_symbol_3);
  gl_Position = inner_result.position;
  color_2 = inner_result.color;
  quad_pos_2 = inner_result.quad_pos;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

struct TestData {
  int dmat2atxa2[4];
};

vec2 rand_seed = vec2(0.0f, 0.0f);
struct RenderParams {
  mat4 modelViewProjectionMatrix;
  vec3 right;
  vec3 up;
};

struct VertexInput {
  vec3 position;
  vec4 color;
  vec2 quad_pos;
};

struct VertexOutput {
  vec4 position;
  vec4 color;
  vec2 quad_pos;
};

struct SimulationParams {
  float deltaTime;
  uint pad;
  uint pad_1;
  uint pad_2;
  vec4 seed;
};

struct Particle {
  vec3 position;
  float lifetime;
  vec4 color;
  vec2 velocity;
  uint pad;
  uint pad_1;
};

layout(binding = 0, std140) uniform sim_params_block_ubo {
  SimulationParams inner;
} sim_params;

layout(binding = 1, std430) buffer Particles_ssbo {
  Particle particles[];
} data;

struct UBO {
  uint width;
};

void assign_and_preserve_padding_data_particles_X(uint dest[1], Particle value) {
  data.particles[dest[0]].position = value.position;
  data.particles[dest[0]].lifetime = value.lifetime;
  data.particles[dest[0]].color = value.color;
  data.particles[dest[0]].velocity = value.velocity;
}

void simulate(uvec3 GlobalInvocationID) {
  rand_seed = ((sim_params.inner.seed.xy * vec2(GlobalInvocationID.xy)) * sim_params.inner.seed.zw);
  uint idx = GlobalInvocationID.x;
  Particle particle = data.particles[idx];
  uint tint_symbol_3[1] = uint[1](idx);
  assign_and_preserve_padding_data_particles_X(tint_symbol_3, particle);
}

layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;
void main() {
  simulate(gl_GlobalInvocationID);
  return;
}
#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

float tint_float_modulo(float lhs, float rhs) {
  return (lhs - rhs * trunc(lhs / rhs));
}


struct TestData {
  int dmat2atxa2[4];
};

struct RenderParams {
  mat4 modelViewProjectionMatrix;
  vec3 right;
  vec3 up;
};

struct VertexInput {
  vec3 position;
  vec4 color;
  vec2 quad_pos;
};

struct VertexOutput {
  vec4 position;
  vec4 color;
  vec2 quad_pos;
};

struct SimulationParams {
  float deltaTime;
  vec4 seed;
};

struct Particle {
  vec3 position;
  float lifetime;
  vec4 color;
  vec2 velocity;
};

struct UBO {
  uint width;
};

layout(binding = 3, std140) uniform ubo_block_ubo {
  UBO inner;
} ubo;

layout(binding = 4, std430) buffer Buffer_ssbo {
  float weights[];
} buf_in;

layout(binding = 5, std430) buffer Buffer_ssbo_1 {
  float weights[];
} buf_out;

layout(binding = 7, rgba8) uniform highp writeonly image2D tex_out;
void export_level(uvec3 coord) {
  if (all(lessThan(coord.xy, uvec2(uvec2(imageSize(tex_out)))))) {
    uint dst_offset = (coord.x << ((coord.y * ubo.inner.width) & 31u));
    uint src_offset = ((coord.x - 2u) + ((coord.y >> 2u) * ubo.inner.width));
    float a = buf_in.weights[(src_offset << 0u)];
    float b = buf_in.weights[(src_offset + 1u)];
    float c = buf_in.weights[((src_offset + 1u) + ubo.inner.width)];
    float d = buf_in.weights[((src_offset + 1u) + ubo.inner.width)];
    float sum = dot(vec4(a, b, c, d), vec4(1.0f));
    buf_out.weights[dst_offset] = tint_float_modulo(sum, 4.0f);
    vec4 probabilities = (vec4(a, (a * b), ((a / b) + c), sum) + max(sum, 0.0f));
    imageStore(tex_out, ivec2(coord.xy), probabilities);
  }
}

layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;
void main() {
  export_level(gl_GlobalInvocationID);
  return;
}
