@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

@group(1) @binding(0) var arg_0 : texture_cube<f32, filterable>;

@group(1) @binding(1) var arg_1 : sampler<filtering>;

fn textureSample_b4951a() -> vec4<f32> {
  var res : vec4<f32> = textureSample(arg_0, arg_1, vec3<f32>(1.0f));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = textureSample_b4951a();
}
