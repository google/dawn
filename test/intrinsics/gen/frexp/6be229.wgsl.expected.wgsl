var<workgroup> arg_1 : vec3<u32>;

fn frexp_6be229() {
  var res : vec3<f32> = frexp(vec3<f32>(), &(arg_1));
}

[[stage(compute)]]
fn compute_main() {
  frexp_6be229();
}
