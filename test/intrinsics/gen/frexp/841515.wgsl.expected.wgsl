var<workgroup> arg_1 : vec4<u32>;

fn frexp_841515() {
  var res : vec4<f32> = frexp(vec4<f32>(), &(arg_1));
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  frexp_841515();
}
