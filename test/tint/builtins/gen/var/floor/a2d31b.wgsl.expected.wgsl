enable f16;

fn floor_a2d31b() {
  var arg_0 = vec4<f16>(f16());
  var res : vec4<f16> = floor(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  floor_a2d31b();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  floor_a2d31b();
}

@compute @workgroup_size(1)
fn compute_main() {
  floor_a2d31b();
}
