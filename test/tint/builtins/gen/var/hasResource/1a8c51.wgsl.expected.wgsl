enable chromium_experimental_resource_table;

@group(0) @binding(0) var<storage, read_write> prevent_dce : i32;

fn hasResource_1a8c51() -> i32 {
  var arg_0 = 1u;
  var res : bool = hasResource<sampler_comparison>(arg_0);
  return select(0, 1, all((res == bool())));
}

@fragment
fn fragment_main() {
  prevent_dce = hasResource_1a8c51();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = hasResource_1a8c51();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : i32,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = hasResource_1a8c51();
  return out;
}
