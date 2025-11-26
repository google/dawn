@group(0) @binding(0) var<storage, read_write> prevent_dce : i32;

fn hasResource_be94be() -> i32 {
  var arg_0 = 1u;
  var res : bool = hasResource<texture_1d<i32>>(arg_0);
  return select(0, 1, all((res == bool())));
}

@fragment
fn fragment_main() {
  prevent_dce = hasResource_be94be();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = hasResource_be94be();
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
  out.prevent_dce = hasResource_be94be();
  return out;
}
