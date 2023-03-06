@group(1) @binding(1) var arg_1 : texture_2d<u32>;

@group(1) @binding(2) var arg_2 : sampler;

fn textureGather_49b07f() {
  var res : vec4<u32> = textureGather(1i, arg_1, arg_2, vec2<f32>(1.0f), vec2<i32>(1i));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<u32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureGather_49b07f();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureGather_49b07f();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureGather_49b07f();
}
