@group(1) @binding(0) var arg_0 : texture_multisampled_2d<u32>;

fn textureLoad_49f76f() {
  var arg_1 = vec2<u32>();
  var arg_2 = 1u;
  var res : vec4<u32> = textureLoad(arg_0, arg_1, arg_2);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureLoad_49f76f();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureLoad_49f76f();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureLoad_49f76f();
}
