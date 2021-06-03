[[group(1), binding(0)]] var arg_0 : [[access(read)]] texture_storage_2d<rgba16sint>;

fn textureDimensions_d4106f() {
  var res : vec2<i32> = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() {
  textureDimensions_d4106f();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_d4106f();
}

[[stage(compute)]]
fn compute_main() {
  textureDimensions_d4106f();
}
