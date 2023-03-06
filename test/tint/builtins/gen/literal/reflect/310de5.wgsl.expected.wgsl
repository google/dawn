enable f16;

fn reflect_310de5() {
  var res : vec4<f16> = reflect(vec4<f16>(1.0h), vec4<f16>(1.0h));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<f16>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  reflect_310de5();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  reflect_310de5();
}

@compute @workgroup_size(1)
fn compute_main() {
  reflect_310de5();
}
