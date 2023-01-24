alias a = i32;
alias a__ = i32;
alias b = a;
alias b__ = a__;

fn f() {
    var c : b;
    var d : b__;
}
