[[stage(compute), workgroup_size(1)]]
fn main() {
  let res = frexp(1.230000019);
  let exp : i32 = res.exp;
  let sig : f32 = res.sig;
}
