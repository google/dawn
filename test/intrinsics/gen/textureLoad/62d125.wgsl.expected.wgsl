[[group(1), binding(0)]] var arg_0 : [[access(read)]] texture_storage_3d<rgba8snorm>;

fn textureLoad_62d125() {
  var res : vec4<f32> = textureLoad(arg_0, vec3<i32>());
}

[[stage(vertex)]]
fn vertex_main() {
  textureLoad_62d125();
}

[[stage(fragment)]]
fn fragment_main() {
  textureLoad_62d125();
}

[[stage(compute)]]
fn compute_main() {
  textureLoad_62d125();
}
