// flags: --rename-all

@compute @workgroup_size(1)
fn i(){let s=modf(1.).whole;}
