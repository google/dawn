fn c(z: i32) -> i32 {
    var a = 1 + z;
    a = a + 2;
    return a;
}

@compute @workgroup_size(1)
fn b() {
    var b = c(2);
    b += c(3);
}
