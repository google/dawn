@group(1) @binding(0) var arg_0 : texture_storage_3d<rg32float, write>;

fn textureDimensions_63f3cf() {
  var res : vec3<i32> = textureDimensions(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_63f3cf();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_63f3cf();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_63f3cf();
}
