[[group(1), binding(0)]] var arg_0 : [[access(write)]] texture_storage_1d<rgba16uint>;

fn textureStore_2eb2a4() {
  textureStore(arg_0, 1, vec4<u32>());
}

[[stage(vertex)]]
fn vertex_main() {
  textureStore_2eb2a4();
}

[[stage(fragment)]]
fn fragment_main() {
  textureStore_2eb2a4();
}

[[stage(compute)]]
fn compute_main() {
  textureStore_2eb2a4();
}
