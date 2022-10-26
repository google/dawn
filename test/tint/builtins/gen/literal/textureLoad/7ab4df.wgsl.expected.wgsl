@group(1) @binding(0) var arg_0 : texture_2d_array<u32>;

fn textureLoad_7ab4df() {
  var res : vec4<u32> = textureLoad(arg_0, vec2<i32>(), 1u, 1i);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureLoad_7ab4df();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureLoad_7ab4df();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureLoad_7ab4df();
}
