var<private> arr = array(1.0f, 2.0f);

@compute @workgroup_size(1)
fn f() {
  var v = arr;
}
