[[group(1), binding(0)]] var arg_0 : [[access(read)]] texture_storage_2d<rgba32sint>;

fn textureLoad_dbd554() {
  var res : vec4<i32> = textureLoad(arg_0, vec2<i32>());
}

[[stage(vertex)]]
fn vertex_main() {
  textureLoad_dbd554();
}

[[stage(fragment)]]
fn fragment_main() {
  textureLoad_dbd554();
}

[[stage(compute)]]
fn compute_main() {
  textureLoad_dbd554();
}
