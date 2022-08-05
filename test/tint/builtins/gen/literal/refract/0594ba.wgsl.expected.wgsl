enable f16;

fn refract_0594ba() {
  var res : vec4<f16> = refract(vec4<f16>(f16()), vec4<f16>(f16()), f16());
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  refract_0594ba();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  refract_0594ba();
}

@compute @workgroup_size(1)
fn compute_main() {
  refract_0594ba();
}
