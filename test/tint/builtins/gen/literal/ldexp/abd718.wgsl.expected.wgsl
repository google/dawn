fn ldexp_abd718() {
  var res : vec2<f32> = ldexp(vec2<f32>(1.0f), vec2<i32>(1i));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  ldexp_abd718();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  ldexp_abd718();
}

@compute @workgroup_size(1)
fn compute_main() {
  ldexp_abd718();
}
