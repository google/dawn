enable f16;

fn clamp_2c251b() {
  var res : vec4<f16> = clamp(vec4<f16>(1.0h), vec4<f16>(1.0h), vec4<f16>(1.0h));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  clamp_2c251b();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  clamp_2c251b();
}

@compute @workgroup_size(1)
fn compute_main() {
  clamp_2c251b();
}
