@group(1) @binding(0) var arg_0 : texture_storage_1d<rgba8sint, write>;

fn textureDimensions_9da9e2() {
  var res : i32 = textureDimensions(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_9da9e2();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_9da9e2();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_9da9e2();
}
