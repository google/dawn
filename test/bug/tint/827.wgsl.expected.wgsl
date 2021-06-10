bug/tint/827.wgsl:8:26 warning: use of deprecated language feature: declare access with var<storage, read_write> instead of using [[access]] decoration
[[group(0), binding(1)]] var<storage> result : [[access(read_write)]] Result;
                         ^^^

[[block]]
struct Result {
  values : array<f32>;
};

let width : u32 = 128u;

[[group(0), binding(0)]] var tex : texture_depth_2d;

[[group(0), binding(1)]] var<storage, read_write> result : Result;

[[stage(compute)]]
fn main([[builtin(global_invocation_id)]] GlobalInvocationId : vec3<u32>) {
  result.values[((GlobalInvocationId.y * width) + GlobalInvocationId.x)] = textureLoad(tex, vec2<i32>(i32(GlobalInvocationId.x), i32(GlobalInvocationId.y)), 0);
}
