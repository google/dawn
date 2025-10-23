fn foo(a: bool, b: bool, c: bool, d: bool, e: bool) {
    if a {
        if b {
            return;
        }
        if c {
            if d {
                return;
            }
            if e {
            }
        }
    }
}

@compute @workgroup_size(1)
fn main() {
    foo(true, true, false, false, false);
}
