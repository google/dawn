@group(1) @binding(0) var arg_0 : texture_cube_array<f32>;

@group(1) @binding(1) var arg_1 : sampler;

fn textureSampleGrad_bbb58f() {
  var arg_2 = vec3<f32>(1.0f);
  var arg_3 = 1u;
  var arg_4 = vec3<f32>(1.0f);
  var arg_5 = vec3<f32>(1.0f);
  var res : vec4<f32> = textureSampleGrad(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureSampleGrad_bbb58f();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureSampleGrad_bbb58f();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureSampleGrad_bbb58f();
}
