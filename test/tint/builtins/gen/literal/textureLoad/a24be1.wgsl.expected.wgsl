@group(1) @binding(0) var arg_0 : texture_2d_array<u32>;

fn textureLoad_a24be1() {
  var res : vec4<u32> = textureLoad(arg_0, vec2<u32>(1u), 1i, 1u);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureLoad_a24be1();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureLoad_a24be1();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureLoad_a24be1();
}
