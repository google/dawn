@group(1) @binding(0) var arg_0 : texture_3d<u32>;

fn textureLoad_a9a9f5() {
  var arg_1 = vec3<i32>();
  var arg_2 = 0;
  var res : vec4<u32> = textureLoad(arg_0, arg_1, arg_2);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureLoad_a9a9f5();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  textureLoad_a9a9f5();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  textureLoad_a9a9f5();
}
