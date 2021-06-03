[[group(1), binding(0)]] var arg_0 : [[access(read)]] texture_storage_3d<rgba8sint>;

fn textureLoad_3d001b() {
  var res : vec4<i32> = textureLoad(arg_0, vec3<i32>());
}

[[stage(vertex)]]
fn vertex_main() {
  textureLoad_3d001b();
}

[[stage(fragment)]]
fn fragment_main() {
  textureLoad_3d001b();
}

[[stage(compute)]]
fn compute_main() {
  textureLoad_3d001b();
}
