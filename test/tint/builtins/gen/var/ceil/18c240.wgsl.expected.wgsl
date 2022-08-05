enable f16;

fn ceil_18c240() {
  var arg_0 = vec2<f16>(f16());
  var res : vec2<f16> = ceil(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  ceil_18c240();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  ceil_18c240();
}

@compute @workgroup_size(1)
fn compute_main() {
  ceil_18c240();
}
