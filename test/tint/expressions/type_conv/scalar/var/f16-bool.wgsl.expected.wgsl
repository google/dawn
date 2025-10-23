enable f16;

var<private> u = f16(1.0h);

@compute @workgroup_size(1)
fn f() {
  let v : bool = bool(u);
}
