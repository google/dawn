@group(1) @binding(0) var arg_0 : texture_2d_array<i32>;

fn textureLoad_79e697() {
  var res : vec4<i32> = textureLoad(arg_0, vec2<i32>(), 1i, 1i);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureLoad_79e697();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureLoad_79e697();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureLoad_79e697();
}
