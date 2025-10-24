var<private> arr = array(1u, 2u);

@compute @workgroup_size(1)
fn f() {
    var v = arr;
}
