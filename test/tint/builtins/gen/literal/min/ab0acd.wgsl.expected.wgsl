enable f16;

fn min_ab0acd() {
  var res : vec3<f16> = min(vec3<f16>(1.0h), vec3<f16>(1.0h));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<f16>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  min_ab0acd();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  min_ab0acd();
}

@compute @workgroup_size(1)
fn compute_main() {
  min_ab0acd();
}
