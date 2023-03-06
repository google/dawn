enable f16;

fn sin_3cca11() {
  var res : vec2<f16> = sin(vec2<f16>(1.5703125h));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<f16>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  sin_3cca11();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  sin_3cca11();
}

@compute @workgroup_size(1)
fn compute_main() {
  sin_3cca11();
}
