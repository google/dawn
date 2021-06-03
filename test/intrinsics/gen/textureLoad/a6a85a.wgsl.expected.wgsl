[[group(1), binding(0)]] var arg_0 : [[access(read)]] texture_storage_3d<rgba8unorm>;

fn textureLoad_a6a85a() {
  var res : vec4<f32> = textureLoad(arg_0, vec3<i32>());
}

[[stage(vertex)]]
fn vertex_main() {
  textureLoad_a6a85a();
}

[[stage(fragment)]]
fn fragment_main() {
  textureLoad_a6a85a();
}

[[stage(compute)]]
fn compute_main() {
  textureLoad_a6a85a();
}
