fn refract_cd905f() {
  var res : vec2<f32> = refract(vec2<f32>(1.0f), vec2<f32>(1.0f), 1.0f);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  refract_cd905f();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  refract_cd905f();
}

@compute @workgroup_size(1)
fn compute_main() {
  refract_cd905f();
}
