@group(0) @binding(0) var<storage, read_write> prevent_dce : i32;

fn hasResource_e4d9ed() -> i32 {
  var arg_0 = 1i;
  var res : bool = hasResource<texture_2d_array<i32>>(arg_0);
  return select(0, 1, all((res == bool())));
}

@fragment
fn fragment_main() {
  prevent_dce = hasResource_e4d9ed();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = hasResource_e4d9ed();
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
  out.prevent_dce = hasResource_e4d9ed();
  return out;
}
