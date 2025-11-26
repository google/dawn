@group(0) @binding(0) var<storage, read_write> prevent_dce : i32;

fn hasResource_ad5652() -> i32 {
  var res : bool = hasResource<texture_3d<i32>>(1i);
  return select(0, 1, all((res == bool())));
}

@fragment
fn fragment_main() {
  prevent_dce = hasResource_ad5652();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = hasResource_ad5652();
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
  out.prevent_dce = hasResource_ad5652();
  return out;
}
