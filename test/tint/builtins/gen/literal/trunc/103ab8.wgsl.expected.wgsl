enable f16;

fn trunc_103ab8() {
  var res : vec3<f16> = trunc(vec3<f16>(1.5h));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<f16>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  trunc_103ab8();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  trunc_103ab8();
}

@compute @workgroup_size(1)
fn compute_main() {
  trunc_103ab8();
}
