enable f16;

fn sin_66a59f() {
  var res : f16 = sin(1.5703125h);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f16;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  sin_66a59f();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  sin_66a59f();
}

@compute @workgroup_size(1)
fn compute_main() {
  sin_66a59f();
}
