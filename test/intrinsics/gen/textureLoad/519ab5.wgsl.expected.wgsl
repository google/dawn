[[group(1), binding(0)]] var arg_0 : [[access(read)]] texture_storage_1d<rgba8unorm>;

fn textureLoad_519ab5() {
  var res : vec4<f32> = textureLoad(arg_0, 1);
}

[[stage(vertex)]]
fn vertex_main() {
  textureLoad_519ab5();
}

[[stage(fragment)]]
fn fragment_main() {
  textureLoad_519ab5();
}

[[stage(compute)]]
fn compute_main() {
  textureLoad_519ab5();
}
