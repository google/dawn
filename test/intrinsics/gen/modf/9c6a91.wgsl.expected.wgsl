intrinsics/gen/modf/9c6a91.wgsl:29:24 warning: use of deprecated intrinsic
  var res: vec2<f32> = modf(vec2<f32>(), &arg_1);
                       ^^^^

var<private> arg_1 : vec2<f32>;

fn modf_9c6a91() {
  var res : vec2<f32> = modf(vec2<f32>(), &(arg_1));
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  modf_9c6a91();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  modf_9c6a91();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  modf_9c6a91();
}
