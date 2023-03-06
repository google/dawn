enable f16;

fn distance_7272f3() {
  var arg_0 = vec4<f16>(1.0h);
  var arg_1 = vec4<f16>(1.0h);
  var res : f16 = distance(arg_0, arg_1);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f16;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  distance_7272f3();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  distance_7272f3();
}

@compute @workgroup_size(1)
fn compute_main() {
  distance_7272f3();
}
