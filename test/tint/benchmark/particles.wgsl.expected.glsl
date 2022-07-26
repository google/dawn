SKIP: FAILED

#version 310 es

layout(location = 0) in vec3 position_1;
layout(location = 1) in vec4 color_1;
layout(location = 2) in vec2 quad_pos_1;
layout(location = 0) out vec4 color_2;
layout(location = 1) out vec2 quad_pos_2;
struct RenderParams {
  mat4 modelViewProjectionMatrix;
  vec3 right;
  vec3 up;
};

layout(binding = 0) uniform RenderParams_1 {
  mat4 modelViewProjectionMatrix;
  vec3 right;
  vec3 up;
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
  vec3 velocity;
};

struct UBO {
  uint width;
};

VertexOutput vs_main(VertexInput tint_symbol) {
  vec3 quad_pos = (mat2x3(render_params.right, render_params.up) * tint_symbol.quad_pos);
  vec3 position = (tint_symbol.position + (quad_pos * 0.01f));
  VertexOutput tint_symbol_1 = VertexOutput(vec4(0.0f, 0.0f, 0.0f, 0.0f), vec4(0.0f, 0.0f, 0.0f, 0.0f), vec2(0.0f, 0.0f));
  tint_symbol_1.position = (render_params.modelViewProjectionMatrix * vec4(position, 1.0f));
  tint_symbol_1.color = tint_symbol.color;
  tint_symbol_1.quad_pos = tint_symbol.quad_pos;
  return tint_symbol_1;
}

void main() {
  gl_PointSize = 1.0;
  VertexInput tint_symbol_2 = VertexInput(position_1, color_1, quad_pos_1);
  VertexOutput inner_result = vs_main(tint_symbol_2);
  gl_Position = inner_result.position;
  color_2 = inner_result.color;
  quad_pos_2 = inner_result.quad_pos;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
#version 310 es
precision mediump float;

layout(location = 0) in vec4 color_1;
layout(location = 1) in vec2 quad_pos_1;
layout(location = 0) out vec4 value;
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
  vec3 velocity;
};

struct UBO {
  uint width;
};

vec4 fs_main(VertexOutput tint_symbol) {
  vec4 color = tint_symbol.color;
  color.a = (color.a * max((1.0f - length(tint_symbol.quad_pos)), 0.0f));
  return color;
}

void main() {
  VertexOutput tint_symbol_1 = VertexOutput(gl_FragCoord, color_1, quad_pos_1);
  vec4 inner_result = fs_main(tint_symbol_1);
  value = inner_result;
  return;
}
#version 310 es

vec2 rand_seed = vec2(0.0f, 0.0f);
float rand() {
  rand_seed.x = fract((cos(dot(rand_seed, vec2(23.140779495f, 232.616897583f))) * 136.816802979f));
  rand_seed.y = fract((cos(dot(rand_seed, vec2(54.478565216f, 345.841522217f))) * 534.764526367f));
  return rand_seed.y;
}

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
  vec3 velocity;
};

layout(binding = 0) uniform SimulationParams_1 {
  float deltaTime;
  vec4 seed;
} sim_params;

layout(binding = 1, std430) buffer Particles_1 {
  Particle particles[];
} data;
struct UBO {
  uint width;
};

uniform highp sampler2D tint_symbol_6;
void simulate(uvec3 GlobalInvocationID) {
  rand_seed = ((sim_params.seed.xy + vec2(GlobalInvocationID.xy)) * sim_params.seed.zw);
  uint idx = GlobalInvocationID.x;
  Particle particle = data.particles[idx];
  particle.velocity.z = (particle.velocity.z - (sim_params.deltaTime * 0.5f));
  particle.position = (particle.position + (sim_params.deltaTime * particle.velocity));
  particle.lifetime = (particle.lifetime - sim_params.deltaTime);
  particle.color.a = smoothstep(0.0f, 0.5f, particle.lifetime);
  if ((particle.lifetime < 0.0f)) {
    ivec2 coord = ivec2(0);
    {
      for(int level = (textureQueryLevels(tint_symbol_6) - 1); (level > 0); level = (level - 1)) {
        vec4 probabilites = texelFetch(tint_symbol_6, coord, level);
        float tint_symbol_5 = rand();
        vec4 value = vec4(tint_symbol_5);
        bvec4 mask = bvec4(uvec4(greaterThanEqual(value, vec4(0.0f, probabilites.xyz))) & uvec4(lessThan(value, probabilites)));
        coord = (coord * 2);
        coord.x = (coord.x + (any(mask.yw) ? 1 : 0));
        coord.y = (coord.y + (any(mask.zw) ? 1 : 0));
      }
    }
    vec2 uv = (vec2(coord) / vec2(textureSize(tint_symbol_6, 0)));
    particle.position = vec3((((uv - 0.5f) * 3.0f) * vec2(1.0f, -1.0f)), 0.0f);
    particle.color = texelFetch(tint_symbol_6, coord, 0);
    float tint_symbol_1 = rand();
    particle.velocity.x = ((tint_symbol_1 - 0.5f) * 0.100000001f);
    float tint_symbol_2 = rand();
    particle.velocity.y = ((tint_symbol_2 - 0.5f) * 0.100000001f);
    float tint_symbol_3 = rand();
    particle.velocity.z = (tint_symbol_3 * 0.300000012f);
    float tint_symbol_4 = rand();
    particle.lifetime = (0.5f + (tint_symbol_4 * 2.0f));
  }
  data.particles[idx] = particle;
}

layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;
void main() {
  simulate(gl_GlobalInvocationID);
  return;
}
Error parsing GLSL shader:
ERROR: 0:64: 'textureQueryLevels' : no matching overloaded function found 
ERROR: 0:64: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es

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
  vec3 velocity;
};

struct UBO {
  uint width;
};

layout(binding = 3) uniform UBO_1 {
  uint width;
} ubo;

layout(binding = 4, std430) buffer Buffer_1 {
  float weights[];
} buf_in;
layout(binding = 5, std430) buffer Buffer_2 {
  float weights[];
} buf_out;
uniform highp sampler2D tex_in_1;
void import_level(uvec3 coord) {
  uint offset = (coord.x + (coord.y * ubo.width));
  buf_out.weights[offset] = texelFetch(tex_in_1, ivec2(coord.xy), 0).w;
}

layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;
void main() {
  import_level(gl_GlobalInvocationID);
  return;
}
#version 310 es

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
  vec3 velocity;
};

struct UBO {
  uint width;
};

layout(binding = 3) uniform UBO_1 {
  uint width;
} ubo;

layout(binding = 4, std430) buffer Buffer_1 {
  float weights[];
} buf_in;
layout(binding = 5, std430) buffer Buffer_2 {
  float weights[];
} buf_out;
layout(rgba8) uniform highp writeonly image2D tex_out;
void export_level(uvec3 coord) {
  if (all(lessThan(coord.xy, uvec2(imageSize(tex_out))))) {
    uint dst_offset = (coord.x + (coord.y * ubo.width));
    uint src_offset = ((coord.x * 2u) + ((coord.y * 2u) * ubo.width));
    float a = buf_in.weights[(src_offset + 0u)];
    float b = buf_in.weights[(src_offset + 1u)];
    float c = buf_in.weights[((src_offset + 0u) + ubo.width)];
    float d = buf_in.weights[((src_offset + 1u) + ubo.width)];
    float sum = dot(vec4(a, b, c, d), vec4(1.0f));
    buf_out.weights[dst_offset] = (sum / 4.0f);
    vec4 probabilities = (vec4(a, (a + b), ((a + b) + c), sum) / max(sum, 0.0001f));
    imageStore(tex_out, ivec2(coord.xy), probabilities);
  }
}

layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;
void main() {
  export_level(gl_GlobalInvocationID);
  return;
}
