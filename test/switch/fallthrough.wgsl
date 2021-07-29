[[stage(compute), workgroup_size(1)]]
fn f() {
    var i : i32;
    switch(i) {
        case 0: {
            fallthrough;
        }
        default: {
            break;
        }
    }
}
