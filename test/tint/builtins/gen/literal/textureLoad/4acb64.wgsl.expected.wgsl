@group(1) @binding(0) var arg_0 : texture_2d_array<f32>;

fn textureLoad_4acb64() {
  var res : vec4<f32> = textureLoad(arg_0, vec2<i32>(), 1u, 1i);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureLoad_4acb64();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureLoad_4acb64();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureLoad_4acb64();
}
