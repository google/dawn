enable f16;

fn mix_e46a83() {
  var res : vec2<f16> = mix(vec2<f16>(1.0h), vec2<f16>(1.0h), 1.0h);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  mix_e46a83();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  mix_e46a83();
}

@compute @workgroup_size(1)
fn compute_main() {
  mix_e46a83();
}
