enable f16;

fn distance_928fa0() {
  var arg_0 = vec3<f16>(1.0h);
  var arg_1 = vec3<f16>(1.0h);
  var res : f16 = distance(arg_0, arg_1);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f16;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  distance_928fa0();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  distance_928fa0();
}

@compute @workgroup_size(1)
fn compute_main() {
  distance_928fa0();
}
