[[group(1), binding(0)]] var arg_0 : [[access(read)]] texture_storage_2d_array<rgba8snorm>;

fn textureLoad_072e26() {
  var res : vec4<f32> = textureLoad(arg_0, vec2<i32>(), 1);
}

[[stage(vertex)]]
fn vertex_main() {
  textureLoad_072e26();
}

[[stage(fragment)]]
fn fragment_main() {
  textureLoad_072e26();
}

[[stage(compute)]]
fn compute_main() {
  textureLoad_072e26();
}
