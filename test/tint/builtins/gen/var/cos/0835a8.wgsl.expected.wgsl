enable f16;

fn cos_0835a8() {
  var arg_0 = vec3<f16>(0.0h);
  var res : vec3<f16> = cos(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<f16>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  cos_0835a8();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  cos_0835a8();
}

@compute @workgroup_size(1)
fn compute_main() {
  cos_0835a8();
}
