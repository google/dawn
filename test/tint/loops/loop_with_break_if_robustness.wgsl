fn f() -> i32 {
    var i : i32;
    loop {
        if (i > 4) {
            return i;
        }
        continuing {
            i = i + 1;

            break if i == 4;
        }
    }
    return i;
}

@compute @workgroup_size(1)
fn main() {
    _ = f();
}
