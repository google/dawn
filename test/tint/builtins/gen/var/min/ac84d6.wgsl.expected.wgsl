enable f16;

fn min_ac84d6() {
  var arg_0 = 1.0h;
  var arg_1 = 1.0h;
  var res : f16 = min(arg_0, arg_1);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f16;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  min_ac84d6();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  min_ac84d6();
}

@compute @workgroup_size(1)
fn compute_main() {
  min_ac84d6();
}
