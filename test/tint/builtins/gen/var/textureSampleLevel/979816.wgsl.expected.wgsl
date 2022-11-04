builtins/gen/var/textureSampleLevel/979816.wgsl:29:24 warning: use of deprecated builtin
  var res: vec4<f32> = textureSampleLevel(arg_0, arg_1, arg_2);
                       ^^^^^^^^^^^^^^^^^^

@group(1) @binding(0) var arg_0 : texture_external;

@group(1) @binding(1) var arg_1 : sampler;

fn textureSampleLevel_979816() {
  var arg_2 = vec2<f32>(1.0f);
  var res : vec4<f32> = textureSampleLevel(arg_0, arg_1, arg_2);
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
