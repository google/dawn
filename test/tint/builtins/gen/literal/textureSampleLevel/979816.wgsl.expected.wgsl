builtins/gen/literal/textureSampleLevel/979816.wgsl:28:24 warning: use of deprecated builtin
  var res: vec4<f32> = textureSampleLevel(arg_0, arg_1, vec2<f32>());
                       ^^^^^^^^^^^^^^^^^^

@group(1) @binding(0) var arg_0 : texture_external;

@group(1) @binding(1) var arg_1 : sampler;

fn textureSampleLevel_979816() {
  var res : vec4<f32> = textureSampleLevel(arg_0, arg_1, vec2<f32>());
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureSampleLevel_979816();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureSampleLevel_979816();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureSampleLevel_979816();
}
