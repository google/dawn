enable f16;

fn reflect_61ca21() {
  var res : vec3<f16> = reflect(vec3<f16>(1.0h), vec3<f16>(1.0h));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<f16>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  reflect_61ca21();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  reflect_61ca21();
}

@compute @workgroup_size(1)
fn compute_main() {
  reflect_61ca21();
}
