builtins/gen/literal/smoothstep/cb0bfb.wgsl:28:18 warning: use of deprecated builtin
  var res: f32 = smoothStep(1.0, 1.0, 1.0);
                 ^^^^^^^^^^

fn smoothStep_cb0bfb() {
  var res : f32 = smoothStep(1.0, 1.0, 1.0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  smoothStep_cb0bfb();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  smoothStep_cb0bfb();
}

@compute @workgroup_size(1)
fn compute_main() {
  smoothStep_cb0bfb();
}
