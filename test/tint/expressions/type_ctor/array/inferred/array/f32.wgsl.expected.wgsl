var<private> arr = array(array(1.0f, 2.0f), array(3.0f, 4.0f));

@compute @workgroup_size(1)
fn f() {
  var v = arr;
}
