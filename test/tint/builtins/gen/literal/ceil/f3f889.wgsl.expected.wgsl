enable f16;

fn ceil_f3f889() {
  var res : f16 = ceil(1.5h);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f16;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  ceil_f3f889();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  ceil_f3f889();
}

@compute @workgroup_size(1)
fn compute_main() {
  ceil_f3f889();
}
