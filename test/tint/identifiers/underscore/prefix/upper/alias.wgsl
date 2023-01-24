alias A = i32;
alias _A = i32;
alias B = A;
alias _B = _A;

fn f() {
    var c : B;
    var d : _B;
}
