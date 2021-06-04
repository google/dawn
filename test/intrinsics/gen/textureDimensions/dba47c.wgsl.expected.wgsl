[[group(1), binding(0)]] var arg_0 : [[access(read)]] texture_storage_1d<rgba32uint>;

fn textureDimensions_dba47c() {
  var res : i32 = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureDimensions_dba47c();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_dba47c();
}

[[stage(compute)]]
fn compute_main() {
  textureDimensions_dba47c();
}
