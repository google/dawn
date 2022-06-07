@group(1) @binding(0) var arg_0 : texture_storage_3d<rg32sint, write>;

fn textureDimensions_60bf54() {
  var res : vec3<i32> = textureDimensions(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_60bf54();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_60bf54();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_60bf54();
}
