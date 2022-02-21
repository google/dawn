type A = i32;
type _A = i32;
type B = A;
type _B = _A;

fn f() {
    var c : B;
    var d : _B;
}
