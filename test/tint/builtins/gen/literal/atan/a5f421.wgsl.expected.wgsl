enable f16;

fn atan_a5f421() {
  var res : vec3<f16> = atan(vec3<f16>(1.0h));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<f16>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  atan_a5f421();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  atan_a5f421();
}

@compute @workgroup_size(1)
fn compute_main() {
  atan_a5f421();
}
