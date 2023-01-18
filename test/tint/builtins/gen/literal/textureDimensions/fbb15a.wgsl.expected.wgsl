@group(1) @binding(0) var arg_0 : texture_storage_2d<bgra8unorm, write>;

fn textureDimensions_fbb15a() {
  var res : vec2<u32> = textureDimensions(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_fbb15a();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_fbb15a();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_fbb15a();
}
