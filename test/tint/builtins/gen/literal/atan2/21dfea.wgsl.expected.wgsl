enable f16;

fn atan2_21dfea() {
  var res : vec3<f16> = atan2(vec3<f16>(1.0h), vec3<f16>(1.0h));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<f16>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  atan2_21dfea();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  atan2_21dfea();
}

@compute @workgroup_size(1)
fn compute_main() {
  atan2_21dfea();
}
