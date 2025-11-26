@group(0) @binding(0) var<storage, read_write> prevent_dce : i32;

fn hasResource_aabe5b() -> i32 {
  var arg_0 = 1i;
  var res : bool = hasResource<texture_depth_cube>(arg_0);
  return select(0, 1, all((res == bool())));
}

@fragment
fn fragment_main() {
  prevent_dce = hasResource_aabe5b();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = hasResource_aabe5b();
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
  out.prevent_dce = hasResource_aabe5b();
  return out;
}
