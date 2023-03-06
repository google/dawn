enable f16;

fn select_a081f1() {
  var res : vec4<f16> = select(vec4<f16>(1.0h), vec4<f16>(1.0h), vec4<bool>(true));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<f16>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  select_a081f1();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  select_a081f1();
}

@compute @workgroup_size(1)
fn compute_main() {
  select_a081f1();
}
