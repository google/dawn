[[group(1), binding(0)]] var arg_0 : texture_storage_3d<r32sint, write>;

fn textureDimensions_2f289f() {
  var res : vec3<i32> = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureDimensions_2f289f();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_2f289f();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  textureDimensions_2f289f();
}
