enable f16;

fn mix_98ee3e() {
  var res : vec2<f16> = mix(vec2<f16>(1.0h), vec2<f16>(1.0h), vec2<f16>(1.0h));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<f16>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  mix_98ee3e();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  mix_98ee3e();
}

@compute @workgroup_size(1)
fn compute_main() {
  mix_98ee3e();
}
