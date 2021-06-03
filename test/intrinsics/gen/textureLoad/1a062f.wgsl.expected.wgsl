[[group(1), binding(0)]] var arg_0 : [[access(read)]] texture_storage_2d_array<rgba16float>;

fn textureLoad_1a062f() {
  var res : vec4<f32> = textureLoad(arg_0, vec2<i32>(), 1);
}

[[stage(vertex)]]
fn vertex_main() {
  textureLoad_1a062f();
}

[[stage(fragment)]]
fn fragment_main() {
  textureLoad_1a062f();
}

[[stage(compute)]]
fn compute_main() {
  textureLoad_1a062f();
}
