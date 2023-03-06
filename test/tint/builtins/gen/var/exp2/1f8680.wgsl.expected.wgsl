fn exp2_1f8680() {
  var arg_0 = vec3<f32>(1.0f);
  var res : vec3<f32> = exp2(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  exp2_1f8680();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  exp2_1f8680();
}

@compute @workgroup_size(1)
fn compute_main() {
  exp2_1f8680();
}
