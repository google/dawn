enable f16;

var<private> u = bool(true);

@compute @workgroup_size(1)
fn f() {
  let v : f16 = f16(u);
}
