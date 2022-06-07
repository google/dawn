builtins/gen/literal/smoothstep/c11eef.wgsl:28:24 warning: use of deprecated builtin
  var res: vec2<f32> = smoothStep(vec2<f32>(), vec2<f32>(), vec2<f32>());
                       ^^^^^^^^^^

fn smoothStep_c11eef() {
  var res : vec2<f32> = smoothStep(vec2<f32>(), vec2<f32>(), vec2<f32>());
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  smoothStep_c11eef();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  smoothStep_c11eef();
}

@compute @workgroup_size(1)
fn compute_main() {
  smoothStep_c11eef();
}
