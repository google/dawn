enable f16;

fn log2_fb9f0b() {
  var res : vec2<f16> = log2(vec2<f16>(1.0h));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<f16>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  log2_fb9f0b();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  log2_fb9f0b();
}

@compute @workgroup_size(1)
fn compute_main() {
  log2_fb9f0b();
}
