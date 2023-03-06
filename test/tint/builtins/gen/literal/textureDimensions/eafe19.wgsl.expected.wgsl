@group(1) @binding(0) var arg_0 : texture_depth_2d_array;

fn textureDimensions_eafe19() {
  var res : vec2<u32> = textureDimensions(arg_0, 1u);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<u32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_eafe19();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_eafe19();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_eafe19();
}
