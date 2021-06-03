[[group(1), binding(0)]] var arg_0 : [[access(read)]] texture_storage_2d<rgba16uint>;

fn textureLoad_ecc823() {
  var res : vec4<u32> = textureLoad(arg_0, vec2<i32>());
}

[[stage(vertex)]]
fn vertex_main() {
  textureLoad_ecc823();
}

[[stage(fragment)]]
fn fragment_main() {
  textureLoad_ecc823();
}

[[stage(compute)]]
fn compute_main() {
  textureLoad_ecc823();
}
