var<workgroup> arg_1 : i32;

fn frexp_0da285() {
  var res : f32 = frexp(1.0, &(arg_1));
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  frexp_0da285();
}
