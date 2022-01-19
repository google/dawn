var<private> rand_seed : vec2<f32>;

fn rand() -> f32 {
  rand_seed.x = fract((cos(dot(rand_seed, vec2<f32>(23.140779495, 232.616897583))) * 136.816802979));
  rand_seed.y = fract((cos(dot(rand_seed, vec2<f32>(54.478565216, 345.841522217))) * 534.764526367));
  return rand_seed.y;
}

struct RenderParams {
  modelViewProjectionMatrix : mat4x4<f32>;
  right : vec3<f32>;
  up : vec3<f32>;
}

@binding(0) @group(0) var<uniform> render_params : RenderParams;

struct VertexInput {
  @location(0)
  position : vec3<f32>;
  @location(1)
  color : vec4<f32>;
  @location(2)
  quad_pos : vec2<f32>;
}

struct VertexOutput {
  @builtin(position)
  position : vec4<f32>;
  @location(0)
  color : vec4<f32>;
  @location(1)
  quad_pos : vec2<f32>;
}

@stage(vertex)
fn vs_main(in : VertexInput) -> VertexOutput {
  var quad_pos = (mat2x3<f32>(render_params.right, render_params.up) * in.quad_pos);
  var position = (in.position + (quad_pos * 0.01));
  var out : VertexOutput;
  out.position = (render_params.modelViewProjectionMatrix * vec4<f32>(position, 1.0));
  out.color = in.color;
  out.quad_pos = in.quad_pos;
  return out;
}

@stage(fragment)
fn fs_main(in : VertexOutput) -> @location(0) vec4<f32> {
  var color = in.color;
  color.a = (color.a * max((1.0 - length(in.quad_pos)), 0.0));
  return color;
}

struct SimulationParams {
  deltaTime : f32;
  seed : vec4<f32>;
}

struct Particle {
  position : vec3<f32>;
  lifetime : f32;
  color : vec4<f32>;
  velocity : vec3<f32>;
}

struct Particles {
  particles : array<Particle>;
}

@binding(0) @group(0) var<uniform> sim_params : SimulationParams;

@binding(1) @group(0) var<storage, read_write> data : Particles;

@binding(2) @group(0) var texture : texture_2d<f32>;

@stage(compute) @workgroup_size(64)
fn simulate(@builtin(global_invocation_id) GlobalInvocationID : vec3<u32>) {
  rand_seed = ((sim_params.seed.xy + vec2<f32>(GlobalInvocationID.xy)) * sim_params.seed.zw);
  let idx = GlobalInvocationID.x;
  var particle = data.particles[idx];
  particle.velocity.z = (particle.velocity.z - (sim_params.deltaTime * 0.5));
  particle.position = (particle.position + (sim_params.deltaTime * particle.velocity));
  particle.lifetime = (particle.lifetime - sim_params.deltaTime);
  particle.color.a = smoothStep(0.0, 0.5, particle.lifetime);
  if ((particle.lifetime < 0.0)) {
    var coord = vec2<i32>(0, 0);
    for(var level = (textureNumLevels(texture) - 1); (level > 0); level = (level - 1)) {
      let probabilites = textureLoad(texture, coord, level);
      let value = vec4<f32>(rand());
      let mask = ((value >= vec4<f32>(0.0, probabilites.xyz)) & (value < probabilites));
      coord = (coord * 2);
      coord.x = (coord.x + select(0, 1, any(mask.yw)));
      coord.y = (coord.y + select(0, 1, any(mask.zw)));
    }
    let uv = (vec2<f32>(coord) / vec2<f32>(textureDimensions(texture)));
    particle.position = vec3<f32>((((uv - 0.5) * 3.0) * vec2<f32>(1.0, -1.0)), 0.0);
    particle.color = textureLoad(texture, coord, 0);
    particle.velocity.x = ((rand() - 0.5) * 0.100000001);
    particle.velocity.y = ((rand() - 0.5) * 0.100000001);
    particle.velocity.z = (rand() * 0.300000012);
    particle.lifetime = (0.5 + (rand() * 2.0));
  }
  data.particles[idx] = particle;
}

struct UBO {
  width : u32;
}

struct Buffer {
  weights : array<f32>;
}

@binding(3) @group(0) var<uniform> ubo : UBO;

@binding(4) @group(0) var<storage, read> buf_in : Buffer;

@binding(5) @group(0) var<storage, read_write> buf_out : Buffer;

@binding(6) @group(0) var tex_in : texture_2d<f32>;

@binding(7) @group(0) var tex_out : texture_storage_2d<rgba8unorm, write>;

@stage(compute) @workgroup_size(64)
fn import_level(@builtin(global_invocation_id) coord : vec3<u32>) {
  _ = &(buf_in);
  let offset = (coord.x + (coord.y * ubo.width));
  buf_out.weights[offset] = textureLoad(tex_in, vec2<i32>(coord.xy), 0).w;
}

@stage(compute) @workgroup_size(64)
fn export_level(@builtin(global_invocation_id) coord : vec3<u32>) {
  if (all((coord.xy < vec2<u32>(textureDimensions(tex_out))))) {
    let dst_offset = (coord.x + (coord.y * ubo.width));
    let src_offset = ((coord.x * 2u) + ((coord.y * 2u) * ubo.width));
    let a = buf_in.weights[(src_offset + 0u)];
    let b = buf_in.weights[(src_offset + 1u)];
    let c = buf_in.weights[((src_offset + 0u) + ubo.width)];
    let d = buf_in.weights[((src_offset + 1u) + ubo.width)];
    let sum = dot(vec4<f32>(a, b, c, d), vec4<f32>(1.0));
    buf_out.weights[dst_offset] = (sum / 4.0);
    let probabilities = (vec4<f32>(a, (a + b), ((a + b) + c), sum) / max(sum, 0.0001));
    textureStore(tex_out, vec2<i32>(coord.xy), probabilities);
  }
}
