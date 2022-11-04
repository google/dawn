@group(1) @binding(0) var arg_0 : texture_2d_array<f32>;

fn textureLoad_54a59b() {
  var res : vec4<f32> = textureLoad(arg_0, vec2<u32>(1u), 1i, 1i);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureLoad_54a59b();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureLoad_54a59b();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureLoad_54a59b();
}
