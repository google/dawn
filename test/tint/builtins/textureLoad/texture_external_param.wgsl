@group(1) @binding(0) var arg_0: texture_external;

fn textureLoad2d(texture: texture_external, coords: vec2<i32>) -> vec4<f32> {
  return textureLoad(texture, coords);
}

fn doTextureLoad() {
  var res: vec4<f32> = textureLoad2d(arg_0, vec2<i32>());
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  doTextureLoad();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  doTextureLoad();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  doTextureLoad();
}
