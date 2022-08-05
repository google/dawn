enable f16;

fn dot_cd5a04() {
  var arg_0 = vec2<f16>(f16());
  var arg_1 = vec2<f16>(f16());
  var res : f16 = dot(arg_0, arg_1);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  dot_cd5a04();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  dot_cd5a04();
}

@compute @workgroup_size(1)
fn compute_main() {
  dot_cd5a04();
}
