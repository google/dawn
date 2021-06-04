[[group(1), binding(0)]] var arg_0 : [[access(read)]] texture_storage_2d_array<rgba16uint>;

fn textureDimensions_ae457f() {
  var res : vec2<i32> = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureDimensions_ae457f();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_ae457f();
}

[[stage(compute)]]
fn compute_main() {
  textureDimensions_ae457f();
}
