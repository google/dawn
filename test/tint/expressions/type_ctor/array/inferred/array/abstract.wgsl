var<private> arr = array(array(1, 2), array(3, 4));

@compute @workgroup_size(1)
fn f() {
    var v = arr;
}
