enable f16;

var<private> u = u32(1u);

@compute @workgroup_size(1)
fn f() {
  let v : f16 = f16(u);
}
