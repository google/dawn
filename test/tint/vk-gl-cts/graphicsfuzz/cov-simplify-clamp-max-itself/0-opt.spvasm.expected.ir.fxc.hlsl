SKIP: FAILED


struct buf0 {
  /* @offset(0) */
  sequence : vec4i,
}

@group(0) @binding(0) var<uniform> x_7 : buf0;

var<private> x_GLF_color : vec4f;

fn main_1() {
  var a : vec4i;
  var i : i32;
  var sum : i32;
  a = vec4i();
  i = 0i;
  loop {
    if ((i < (x_7.sequence.w + 1i))) {
    } else {
      break;
    }
    if ((x_7.sequence[clamp(i, x_7.sequence.x, i)] == 1i)) {
      let x_57 = i;
      a[x_57] = 5i;
    } else {
      let x_59 = i;
      a[x_59] = i;
    }

    continuing {
      i = (i + 1i);
    }
  }
  sum = (((a.x + a.y) + a.z) + a.w);
  if ((sum == 10i)) {
    x_GLF_color = vec4f(1.0f, 0.0f, 0.0f, 1.0f);
  } else {
    x_GLF_color = vec4f();
  }
  return;
}

struct main_out {
  @location(0)
  x_GLF_color_1 : vec4f,
}

@fragment
fn main() -> main_out {
  main_1();
  return main_out(x_GLF_color);
}

Failed to generate: :44:9 error: binary: no matching overload for 'operator * (i32, u32)'

9 candidate operators:
 • 'operator * (T  ✓ , T  ✗ ) -> T' where:
      ✓  'T' is 'f32', 'i32', 'u32' or 'f16'
 • 'operator * (vecN<T>  ✗ , T  ✓ ) -> vecN<T>' where:
      ✓  'T' is 'f32', 'i32', 'u32' or 'f16'
 • 'operator * (T  ✓ , vecN<T>  ✗ ) -> vecN<T>' where:
      ✓  'T' is 'f32', 'i32', 'u32' or 'f16'
 • 'operator * (T  ✗ , matNxM<T>  ✗ ) -> matNxM<T>' where:
      ✗  'T' is 'f32' or 'f16'
 • 'operator * (matNxM<T>  ✗ , T  ✗ ) -> matNxM<T>' where:
      ✗  'T' is 'f32' or 'f16'
 • 'operator * (vecN<T>  ✗ , vecN<T>  ✗ ) -> vecN<T>' where:
      ✗  'T' is 'f32', 'i32', 'u32' or 'f16'
 • 'operator * (matCxR<T>  ✗ , vecC<T>  ✗ ) -> vecR<T>' where:
      ✗  'T' is 'f32' or 'f16'
 • 'operator * (vecR<T>  ✗ , matCxR<T>  ✗ ) -> vecC<T>' where:
      ✗  'T' is 'f32' or 'f16'
 • 'operator * (matKxR<T>  ✗ , matCxK<T>  ✗ ) -> matCxR<T>' where:
      ✗  'T' is 'f32' or 'f16'

        %20:u32 = mul %19, 4u
        ^^^^^^^^^^^^^^^^^^^^^

:22:7 note: in block
      $B3: {  # body
      ^^^

note: # Disassembly
buf0 = struct @align(16) {
  sequence:vec4<i32> @offset(0)
}

main_out = struct @align(16) {
  x_GLF_color_1:vec4<f32> @offset(0), @location(0)
}

$B1: {  # root
  %x_7:ptr<uniform, array<vec4<u32>, 1>, read> = var @binding_point(0, 0)
  %x_GLF_color:ptr<private, vec4<f32>, read_write> = var
}

%main_1 = func():void {
  $B2: {
    %a:ptr<function, vec4<i32>, read_write> = var
    %i:ptr<function, i32, read_write> = var
    %sum:ptr<function, i32, read_write> = var
    store %a, vec4<i32>(0i)
    store %i, 0i
    loop [b: $B3, c: $B4] {  # loop_1
      $B3: {  # body
        %7:i32 = load %i
        %8:ptr<uniform, vec4<u32>, read> = access %x_7, 0u
        %9:u32 = load_vector_element %8, 3u
        %10:i32 = bitcast %9
        %11:i32 = add %10, 1i
        %12:bool = lt %7, %11
        if %12 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_if  # if_1
          }
          $B6: {  # false
            exit_loop  # loop_1
          }
        }
        %13:i32 = load %i
        %14:ptr<uniform, vec4<u32>, read> = access %x_7, 0u
        %15:u32 = load_vector_element %14, 0u
        %16:i32 = bitcast %15
        %17:i32 = load %i
        %18:i32 = max %13, %16
        %19:i32 = min %18, %17
        %20:u32 = mul %19, 4u
        %21:u32 = div %20, 16u
        %22:ptr<uniform, vec4<u32>, read> = access %x_7, %21
        %23:u32 = mod %20, 16u
        %24:u32 = div %23, 4u
        %25:u32 = load_vector_element %22, %24
        %26:i32 = bitcast %25
        %27:bool = eq %26, 1i
        if %27 [t: $B7, f: $B8] {  # if_2
          $B7: {  # true
            %28:i32 = load %i
            %x_57:i32 = let %28
            store_vector_element %a, %x_57, 5i
            exit_if  # if_2
          }
          $B8: {  # false
            %30:i32 = load %i
            %x_59:i32 = let %30
            %32:i32 = load %i
            store_vector_element %a, %x_59, %32
            exit_if  # if_2
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %33:i32 = load %i
        %34:i32 = add %33, 1i
        store %i, %34
        next_iteration  # -> $B3
      }
    }
    %35:i32 = load_vector_element %a, 0u
    %36:i32 = load_vector_element %a, 1u
    %37:i32 = add %35, %36
    %38:i32 = load_vector_element %a, 2u
    %39:i32 = add %37, %38
    %40:i32 = load_vector_element %a, 3u
    %41:i32 = add %39, %40
    store %sum, %41
    %42:i32 = load %sum
    %43:bool = eq %42, 10i
    if %43 [t: $B9, f: $B10] {  # if_3
      $B9: {  # true
        store %x_GLF_color, vec4<f32>(1.0f, 0.0f, 0.0f, 1.0f)
        exit_if  # if_3
      }
      $B10: {  # false
        store %x_GLF_color, vec4<f32>(0.0f)
        exit_if  # if_3
      }
    }
    ret
  }
}
%main = @fragment func():main_out {
  $B11: {
    %45:void = call %main_1
    %46:vec4<f32> = load %x_GLF_color
    %47:main_out = construct %46
    ret %47
  }
}

