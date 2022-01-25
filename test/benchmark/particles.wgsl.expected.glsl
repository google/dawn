SKIP: FAILED

#version 310 es
precision mediump float;

struct RenderParams {
  mat4 modelViewProjectionMatrix;
  vec3 right;
  vec3 up;
};

layout (binding = 0) uniform RenderParams_1 {
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
struct tint_symbol_4 {
  vec3 position;
  vec4 color;
  vec2 quad_pos;
};
struct tint_symbol_5 {
  vec4 color;
  vec2 quad_pos;
  vec4 position;
};

VertexOutput vs_main_inner(VertexInput tint_symbol) {
  vec3 quad_pos = (mat2x3(render_params.right, render_params.up) * tint_symbol.quad_pos);
  vec3 position = (tint_symbol.position + (quad_pos * 0.01f));
  VertexOutput tint_symbol_1 = VertexOutput(vec4(0.0f, 0.0f, 0.0f, 0.0f), vec4(0.0f, 0.0f, 0.0f, 0.0f), vec2(0.0f, 0.0f));
  tint_symbol_1.position = (render_params.modelViewProjectionMatrix * vec4(position, 1.0f));
  tint_symbol_1.color = tint_symbol.color;
  tint_symbol_1.quad_pos = tint_symbol.quad_pos;
  return tint_symbol_1;
}

struct tint_symbol_7 {
  vec4 color;
  vec2 quad_pos;
  vec4 position;
};
struct tint_symbol_8 {
  vec4 value;
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
struct tint_symbol_10 {
  uvec3 GlobalInvocationID;
};
struct UBO {
  uint width;
};
struct tint_symbol_12 {
  uvec3 coord;
};
struct tint_symbol_14 {
  uvec3 coord;
};

tint_symbol_5 vs_main(tint_symbol_4 tint_symbol_3) {
  VertexInput tint_symbol_15 = VertexInput(tint_symbol_3.position, tint_symbol_3.color, tint_symbol_3.quad_pos);
  VertexOutput inner_result = vs_main_inner(tint_symbol_15);
  tint_symbol_5 wrapper_result = tint_symbol_5(vec4(0.0f, 0.0f, 0.0f, 0.0f), vec2(0.0f, 0.0f), vec4(0.0f, 0.0f, 0.0f, 0.0f));
  wrapper_result.position = inner_result.position;
  wrapper_result.color = inner_result.color;
  wrapper_result.quad_pos = inner_result.quad_pos;
  return wrapper_result;
}
in vec3 position;
in vec4 color;
in vec2 quad_pos;
out vec4 color;
out vec2 quad_pos;
void main() {
  tint_symbol_4 inputs;
  inputs.position = position;
  inputs.color = color;
  inputs.quad_pos = quad_pos;
  tint_symbol_5 outputs;
  outputs = vs_main(inputs);
  color = outputs.color;
  quad_pos = outputs.quad_pos;
  gl_Position = outputs.position;
  gl_Position.y = -gl_Position.y;
}


Error parsing GLSL shader:
ERROR: 0:90: 'color' : redefinition 
ERROR: 1 compilation errors.  No code generated.



#version 310 es
precision mediump float;

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
struct tint_symbol_4 {
  vec3 position;
  vec4 color;
  vec2 quad_pos;
};
struct tint_symbol_5 {
  vec4 color;
  vec2 quad_pos;
  vec4 position;
};
struct tint_symbol_7 {
  vec4 color;
  vec2 quad_pos;
  vec4 position;
};
struct tint_symbol_8 {
  vec4 value;
};

vec4 fs_main_inner(VertexOutput tint_symbol) {
  vec4 color = tint_symbol.color;
  color.a = (color.a * max((1.0f - length(tint_symbol.quad_pos)), 0.0f));
  return color;
}

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
struct tint_symbol_10 {
  uvec3 GlobalInvocationID;
};
struct UBO {
  uint width;
};
struct tint_symbol_12 {
  uvec3 coord;
};
struct tint_symbol_14 {
  uvec3 coord;
};

tint_symbol_8 fs_main(tint_symbol_7 tint_symbol_6) {
  VertexOutput tint_symbol_15 = VertexOutput(tint_symbol_6.position, tint_symbol_6.color, tint_symbol_6.quad_pos);
  vec4 inner_result_1 = fs_main_inner(tint_symbol_15);
  tint_symbol_8 wrapper_result_1 = tint_symbol_8(vec4(0.0f, 0.0f, 0.0f, 0.0f));
  wrapper_result_1.value = inner_result_1;
  return wrapper_result_1;
}
in vec4 color;
in vec2 quad_pos;
out vec4 value;
void main() {
  tint_symbol_7 inputs;
  inputs.color = color;
  inputs.quad_pos = quad_pos;
  inputs.position = gl_FragCoord;
  tint_symbol_8 outputs;
  outputs = fs_main(inputs);
  value = outputs.value;
}


#version 310 es
precision mediump float;

vec2 rand_seed = vec2(0.0f, 0.0f);

float rand() {
  rand_seed.x = frac((cos(dot(rand_seed, vec2(23.140779495f, 232.616897583f))) * 136.816802979f));
  rand_seed.y = frac((cos(dot(rand_seed, vec2(54.478565216f, 345.841522217f))) * 534.764526367f));
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
struct tint_symbol_4 {
  vec3 position;
  vec4 color;
  vec2 quad_pos;
};
struct tint_symbol_5 {
  vec4 color;
  vec2 quad_pos;
  vec4 position;
};
struct tint_symbol_7 {
  vec4 color;
  vec2 quad_pos;
  vec4 position;
};
struct tint_symbol_8 {
  vec4 value;
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

layout (binding = 0) uniform SimulationParams_1 {
  float deltaTime;
  vec4 seed;
} sim_params;
layout (binding = 1) buffer Particles_1 {
  Particle particles[];
} data;

struct tint_symbol_10 {
  uvec3 GlobalInvocationID;
};

uniform highp sampler2D tint_symbol_2_1;

void simulate_inner(uvec3 GlobalInvocationID) {
  rand_seed = ((sim_params.seed.xy + vec2(GlobalInvocationID.xy)) * sim_params.seed.zw);
  uint idx = GlobalInvocationID.x;
  Particle particle = data.particles[idx];
  particle.velocity.z = (particle.velocity.z - (sim_params.deltaTime * 0.5f));
  particle.position = (particle.position + (sim_params.deltaTime * particle.velocity));
  particle.lifetime = (particle.lifetime - sim_params.deltaTime);
  particle.color.a = smoothstep(0.0f, 0.5f, particle.lifetime);
  if ((particle.lifetime < 0.0f)) {
    ivec2 coord = ivec2(0, 0);
    {
      for(int level = (textureQueryLevels(tint_symbol_2_1); - 1); (level > 0); level = (level - 1)) {
        vec4 probabilites = texelFetch(tint_symbol_2_1, coord, level);
        vec4 value = vec4(rand());
        bvec4 mask = (greaterThanEqual(value, vec4(0.0f, probabilites.xyz)) & lessThan(value, probabilites));
        coord = (coord * 2);
        coord.x = (coord.x + (any(mask.yw) ? 1 : 0));
        coord.y = (coord.y + (any(mask.zw) ? 1 : 0));
      }
    }
    vec2 uv = (vec2(coord) / vec2(textureSize(tint_symbol_2_1, 0)));
    particle.position = vec3((((uv - 0.5f) * 3.0f) * vec2(1.0f, -1.0f)), 0.0f);
    particle.color = texelFetch(tint_symbol_2_1, coord, 0);
    particle.velocity.x = ((rand() - 0.5f) * 0.100000001f);
    particle.velocity.y = ((rand() - 0.5f) * 0.100000001f);
    particle.velocity.z = (rand() * 0.300000012f);
    particle.lifetime = (0.5f + (rand() * 2.0f));
  }
  data.particles[idx] = particle;
}

struct UBO {
  uint width;
};
struct tint_symbol_12 {
  uvec3 coord;
};
struct tint_symbol_14 {
  uvec3 coord;
};

layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;
void simulate(tint_symbol_10 tint_symbol_9) {
  simulate_inner(tint_symbol_9.GlobalInvocationID);
  return;
}
void main() {
  tint_symbol_10 inputs;
  inputs.GlobalInvocationID = gl_GlobalInvocationID;
  simulate(inputs);
}


Error parsing GLSL shader:
ERROR: 0:7: 'frac' : no matching overloaded function found 
ERROR: 0:7: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es
precision mediump float;

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
struct tint_symbol_4 {
  vec3 position;
  vec4 color;
  vec2 quad_pos;
};
struct tint_symbol_5 {
  vec4 color;
  vec2 quad_pos;
  vec4 position;
};
struct tint_symbol_7 {
  vec4 color;
  vec2 quad_pos;
  vec4 position;
};
struct tint_symbol_8 {
  vec4 value;
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
struct tint_symbol_10 {
  uvec3 GlobalInvocationID;
};
struct UBO {
  uint width;
};

layout (binding = 3) uniform UBO_1 {
  uint width;
} ubo;
layout (binding = 4) buffer Buffer_1 {
  float weights[];
} buf_in;
layout (binding = 5) buffer Buffer_2 {
  float weights[];
} buf_out;

struct tint_symbol_12 {
  uvec3 coord;
};

uniform highp sampler2D tex_in_1;

void import_level_inner(uvec3 coord) {
  uint offset = (coord.x + (coord.y * ubo.width));
  buf_out.weights[offset] = texelFetch(tex_in_1, ivec2(coord.xy), 0).w;
}

struct tint_symbol_14 {
  uvec3 coord;
};

layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;
void import_level(tint_symbol_12 tint_symbol_11) {
  import_level_inner(tint_symbol_11.coord);
  return;
}
void main() {
  tint_symbol_12 inputs;
  inputs.coord = gl_GlobalInvocationID;
  import_level(inputs);
}


#version 310 es
precision mediump float;

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
struct tint_symbol_4 {
  vec3 position;
  vec4 color;
  vec2 quad_pos;
};
struct tint_symbol_5 {
  vec4 color;
  vec2 quad_pos;
  vec4 position;
};
struct tint_symbol_7 {
  vec4 color;
  vec2 quad_pos;
  vec4 position;
};
struct tint_symbol_8 {
  vec4 value;
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
struct tint_symbol_10 {
  uvec3 GlobalInvocationID;
};
struct UBO {
  uint width;
};

layout (binding = 3) uniform UBO_1 {
  uint width;
} ubo;
layout (binding = 4) buffer Buffer_1 {
  float weights[];
} buf_in;
layout (binding = 5) buffer Buffer_2 {
  float weights[];
} buf_out;

struct tint_symbol_12 {
  uvec3 coord;
};
struct tint_symbol_14 {
  uvec3 coord;
};

layout(rgba8) uniform highp writeonly image2D tex_out_1;

void export_level_inner(uvec3 coord) {
  if (all(lessThan(coord.xy, uvec2(imageSize(tex_out_1))))) {
    uint dst_offset = (coord.x + (coord.y * ubo.width));
    uint src_offset = ((coord.x * 2u) + ((coord.y * 2u) * ubo.width));
    float a_1 = buf_in.weights[(src_offset + 0u)];
    float b = buf_in.weights[(src_offset + 1u)];
    float c = buf_in.weights[((src_offset + 0u) + ubo.width)];
    float d = buf_in.weights[((src_offset + 1u) + ubo.width)];
    float sum = dot(vec4(a_1, b, c, d), vec4(1.0f));
    buf_out.weights[dst_offset] = (sum / 4.0f);
    vec4 probabilities = (vec4(a_1, (a_1 + b), ((a_1 + b) + c), sum) / max(sum, 0.0001f));
    imageStore(tex_out_1, ivec2(coord.xy), probabilities);
  }
}

layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;
void export_level(tint_symbol_14 tint_symbol_13) {
  export_level_inner(tint_symbol_13.coord);
  return;
}
void main() {
  tint_symbol_14 inputs;
  inputs.coord = gl_GlobalInvocationID;
  export_level(inputs);
}


