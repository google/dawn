bug/tint/1064.wgsl:12:9 warning: use of deprecated language feature: `break` must not be used to exit from a continuing block. Use `break-if` instead.
        break;
        ^^^^^

void main() {
  while (true) {
    if (false) {
    } else {
      break;
    }
    {
      if (true) {
      } else {
        break;
      }
    }
  }
  return;
}
