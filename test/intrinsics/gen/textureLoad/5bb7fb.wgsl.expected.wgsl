[[group(1), binding(0)]] var arg_0 : [[access(read)]] texture_storage_1d<rg32uint>;

fn textureLoad_5bb7fb() {
  var res : vec4<u32> = textureLoad(arg_0, 1);
}

[[stage(vertex)]]
fn vertex_main() {
  textureLoad_5bb7fb();
}

[[stage(fragment)]]
fn fragment_main() {
  textureLoad_5bb7fb();
}

[[stage(compute)]]
fn compute_main() {
  textureLoad_5bb7fb();
}
