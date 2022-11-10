enable f16;

fn abs_538d29() {
  var res : vec4<f16> = abs(vec4<f16>(1.0h));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  abs_538d29();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  abs_538d29();
}

@compute @workgroup_size(1)
fn compute_main() {
  abs_538d29();
}
