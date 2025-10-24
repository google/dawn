var<private> arr = array(1f, 2f);

@compute @workgroup_size(1)
fn f() {
    var v = arr;
}
