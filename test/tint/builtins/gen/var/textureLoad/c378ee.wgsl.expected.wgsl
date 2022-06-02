@group(1) @binding(0) var arg_0 : texture_multisampled_2d<u32>;

fn textureLoad_c378ee() {
  var arg_1 = vec2<i32>();
  var arg_2 = 1;
  var res : vec4<u32> = textureLoad(arg_0, arg_1, arg_2);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureLoad_c378ee();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  textureLoad_c378ee();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  textureLoad_c378ee();
}
