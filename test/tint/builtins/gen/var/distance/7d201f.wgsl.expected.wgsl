enable f16;

fn distance_7d201f() {
  var arg_0 = f16();
  var arg_1 = f16();
  var res : f16 = distance(arg_0, arg_1);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  distance_7d201f();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  distance_7d201f();
}

@compute @workgroup_size(1)
fn compute_main() {
  distance_7d201f();
}
