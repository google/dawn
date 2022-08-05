enable f16;

fn distance_892a5d() {
  var arg_0 = vec2<f16>(f16());
  var arg_1 = vec2<f16>(f16());
  var res : f16 = distance(arg_0, arg_1);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  distance_892a5d();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  distance_892a5d();
}

@compute @workgroup_size(1)
fn compute_main() {
  distance_892a5d();
}
