enable f16;

fn step_07cb06() {
  var res : vec2<f16> = step(vec2<f16>(1.0h), vec2<f16>(1.0h));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<f16>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  step_07cb06();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  step_07cb06();
}

@compute @workgroup_size(1)
fn compute_main() {
  step_07cb06();
}
