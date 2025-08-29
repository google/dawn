enable chromium_experimental_dynamic_binding;

@group(0) @binding(0) var<storage, read_write> prevent_dce : i32;

@group(1) @binding(0) var arg_0 : resource_binding;

fn hasBinding_46feef() -> i32 {
  var arg_1 = 1u;
  var res : bool = hasBinding<texture_cube<i32>>(arg_0, arg_1);
  return select(0, 1, all((res == bool())));
}

@fragment
fn fragment_main() {
  prevent_dce = hasBinding_46feef();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = hasBinding_46feef();
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
  out.prevent_dce = hasBinding_46feef();
  return out;
}
