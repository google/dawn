enable f16;

fn exp2_151a4c() {
  var arg_0 = vec2<f16>(f16());
  var res : vec2<f16> = exp2(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  exp2_151a4c();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  exp2_151a4c();
}

@compute @workgroup_size(1)
fn compute_main() {
  exp2_151a4c();
}
