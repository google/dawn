@group(1) @binding(0) var arg_0 : texture_external;

fn textureLoad_1bfdfb() {
  var arg_1 = vec2<u32>();
  var res : vec4<f32> = textureLoad(arg_0, arg_1);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureLoad_1bfdfb();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureLoad_1bfdfb();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureLoad_1bfdfb();
}
