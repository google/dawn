@group(1) @binding(0) var arg_0 : texture_3d<f32>;

@group(1) @binding(1) var arg_1 : sampler;

fn textureSampleGrad_5884dd() {
  var res : vec4<f32> = textureSampleGrad(arg_0, arg_1, vec3<f32>(1.0f), vec3<f32>(1.0f), vec3<f32>(1.0f), vec3<i32>(1i));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureSampleGrad_5884dd();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureSampleGrad_5884dd();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureSampleGrad_5884dd();
}
