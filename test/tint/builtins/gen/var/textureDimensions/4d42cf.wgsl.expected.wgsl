@group(1) @binding(0) var arg_0 : texture_2d<i32>;

fn textureDimensions_4d42cf() {
  var arg_1 = 1u;
  var res : vec2<i32> = textureDimensions(arg_0, arg_1);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_4d42cf();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_4d42cf();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_4d42cf();
}
