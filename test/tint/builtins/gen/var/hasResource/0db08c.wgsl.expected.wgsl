enable chromium_experimental_resource_table;

@group(0) @binding(0) var<storage, read_write> prevent_dce : i32;

fn hasResource_0db08c() -> i32 {
  var arg_0 = 1i;
  var res : bool = hasResource<texture_cube_array<u32>>(arg_0);
  return select(0, 1, all((res == bool())));
}

@fragment
fn fragment_main() {
  prevent_dce = hasResource_0db08c();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = hasResource_0db08c();
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
  out.prevent_dce = hasResource_0db08c();
  return out;
}
