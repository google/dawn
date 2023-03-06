enable f16;

fn clamp_b195eb() {
  var res : vec3<f16> = clamp(vec3<f16>(1.0h), vec3<f16>(1.0h), vec3<f16>(1.0h));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<f16>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  clamp_b195eb();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  clamp_b195eb();
}

@compute @workgroup_size(1)
fn compute_main() {
  clamp_b195eb();
}
