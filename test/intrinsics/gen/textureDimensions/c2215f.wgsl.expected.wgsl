[[group(1), binding(0)]] var arg_0 : [[access(read)]] texture_storage_2d_array<r32float>;

fn textureDimensions_c2215f() {
  var res : vec2<i32> = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() {
  textureDimensions_c2215f();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_c2215f();
}

[[stage(compute)]]
fn compute_main() {
  textureDimensions_c2215f();
}
