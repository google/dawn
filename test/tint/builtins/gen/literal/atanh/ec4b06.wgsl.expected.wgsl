enable f16;

fn atanh_ec4b06() {
  var res : vec3<f16> = atanh(vec3<f16>(0.5h));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<f16>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  atanh_ec4b06();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  atanh_ec4b06();
}

@compute @workgroup_size(1)
fn compute_main() {
  atanh_ec4b06();
}
