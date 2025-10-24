var<private> arr = array(array(1f, 2f), array(3f, 4f));

@compute @workgroup_size(1)
fn f() {
    var v = arr;
}
