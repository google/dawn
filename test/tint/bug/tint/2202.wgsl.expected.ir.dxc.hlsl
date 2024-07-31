<dawn>/test/tint/bug/tint/2202.wgsl:7:9 warning: code is unreachable
        let _e9 = (vec3<i32>().y >= vec3<i32>().y);
        ^^^^^^^


[numthreads(1, 1, 1)]
void main() {
  {
    while(true) {
      {
        while(true) {
          return;
        }
      }
      /* unreachable */
    }
  }
}

