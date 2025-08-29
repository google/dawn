enable chromium_experimental_dynamic_binding;

@group(0) @binding(0) var<storage, read_write> prevent_dce : i32;

@group(1) @binding(0) var arg_0 : resource_binding;

fn hasBinding_d6d93c() -> i32 {
  var arg_1 = 1u;
  var res : bool = hasBinding<texture_depth_2d_array>(arg_0, arg_1);
  return select(0, 1, all((res == bool())));
}

@fragment
fn fragment_main() {
  prevent_dce = hasBinding_d6d93c();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = hasBinding_d6d93c();
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
  out.prevent_dce = hasBinding_d6d93c();
  return out;
}
