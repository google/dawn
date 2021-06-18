var<workgroup> arg_1 : u32;

fn frexp_8b3191() {
  var res : f32 = frexp(1.0, &(arg_1));
}

[[stage(compute)]]
fn compute_main() {
  frexp_8b3191();
}
