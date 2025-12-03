enable chromium_experimental_resource_table;

@group(0) @binding(0) var<storage, read_write> prevent_dce : i32;

fn hasResource_fa19d1() -> i32 {
  var res : bool = hasResource<texture_multisampled_2d<i32>>(1i);
  return select(0, 1, all((res == bool())));
}

@fragment
fn fragment_main() {
  prevent_dce = hasResource_fa19d1();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = hasResource_fa19d1();
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
  out.prevent_dce = hasResource_fa19d1();
  return out;
}
