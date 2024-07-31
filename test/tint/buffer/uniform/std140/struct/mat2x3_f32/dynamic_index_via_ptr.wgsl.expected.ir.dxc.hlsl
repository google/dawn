SKIP: FAILED


struct Inner {
  @size(64)
  m : mat2x3<f32>,
}

struct Outer {
  a : array<Inner, 4>,
}

@group(0) @binding(0) var<uniform> a : array<Outer, 4>;

var<private> counter = 0;

fn i() -> i32 {
  counter++;
  return counter;
}

@compute @workgroup_size(1)
fn f() {
  let p_a = &(a);
  let p_a_i = &((*(p_a))[i()]);
  let p_a_i_a = &((*(p_a_i)).a);
  let p_a_i_a_i = &((*(p_a_i_a))[i()]);
  let p_a_i_a_i_m = &((*(p_a_i_a_i)).m);
  let p_a_i_a_i_m_i = &((*(p_a_i_a_i_m))[i()]);
  let l_a : array<Outer, 4> = *(p_a);
  let l_a_i : Outer = *(p_a_i);
  let l_a_i_a : array<Inner, 4> = *(p_a_i_a);
  let l_a_i_a_i : Inner = *(p_a_i_a_i);
  let l_a_i_a_i_m : mat2x3<f32> = *(p_a_i_a_i_m);
  let l_a_i_a_i_m_i : vec3<f32> = *(p_a_i_a_i_m_i);
  let l_a_i_a_i_m_i_i : f32 = (*(p_a_i_a_i_m_i))[i()];
}

Failed to generate: :38:24 error: binary: %23 is not in scope
    %22:u32 = add %21, %23
                       ^^^

:24:3 note: in block
  $B3: {
  ^^^

:63:5 note: %23 declared here
    %23:u32 = mul %51, 4u
    ^^^^^^^

:43:24 error: binary: %23 is not in scope
    %29:u32 = add %28, %23
                       ^^^

:24:3 note: in block
  $B3: {
  ^^^

:63:5 note: %23 declared here
    %23:u32 = mul %51, 4u
    ^^^^^^^

:48:24 error: binary: %23 is not in scope
    %35:u32 = add %34, %23
                       ^^^

:24:3 note: in block
  $B3: {
  ^^^

:63:5 note: %23 declared here
    %23:u32 = mul %51, 4u
    ^^^^^^^

:63:5 error: binary: no matching overload for 'operator * (i32, u32)'

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

    %23:u32 = mul %51, 4u
    ^^^^^^^^^^^^^^^^^^^^^

:24:3 note: in block
  $B3: {
  ^^^

note: # Disassembly
Inner = struct @align(16) {
  m:mat2x3<f32> @offset(0)
}

Outer = struct @align(16) {
  a:array<Inner, 4> @offset(0)
}

$B1: {  # root
  %a:ptr<uniform, array<vec4<u32>, 64>, read> = var @binding_point(0, 0)
  %counter:ptr<private, i32, read_write> = var, 0i
}

%i = func():i32 {
  $B2: {
    %4:i32 = load %counter
    %5:i32 = add %4, 1i
    store %counter, %5
    %6:i32 = load %counter
    ret %6
  }
}
%f = @compute @workgroup_size(1, 1, 1) func():void {
  $B3: {
    %8:i32 = call %i
    %9:u32 = convert %8
    %10:u32 = mul 256u, %9
    %11:i32 = call %i
    %12:u32 = convert %11
    %13:u32 = mul 64u, %12
    %14:i32 = call %i
    %15:u32 = convert %14
    %16:u32 = mul 16u, %15
    %17:array<Outer, 4> = call %18, 0u
    %l_a:array<Outer, 4> = let %17
    %20:u32 = add %10, %13
    %21:u32 = add %20, %16
    %22:u32 = add %21, %23
    %24:Outer = call %25, %22
    %l_a_i:Outer = let %24
    %27:u32 = add %10, %13
    %28:u32 = add %27, %16
    %29:u32 = add %28, %23
    %30:array<Inner, 4> = call %31, %29
    %l_a_i_a:array<Inner, 4> = let %30
    %33:u32 = add %10, %13
    %34:u32 = add %33, %16
    %35:u32 = add %34, %23
    %36:Inner = call %37, %35
    %l_a_i_a_i:Inner = let %36
    %39:u32 = add %10, %13
    %40:mat2x3<f32> = call %41, %39
    %l_a_i_a_i_m:mat2x3<f32> = let %40
    %43:u32 = add %10, %13
    %44:u32 = add %43, %16
    %45:u32 = div %44, 16u
    %46:ptr<uniform, vec4<u32>, read> = access %a, %45
    %47:vec4<u32> = load %46
    %48:vec3<u32> = swizzle %47, xyz
    %49:vec3<f32> = bitcast %48
    %l_a_i_a_i_m_i:vec3<f32> = let %49
    %51:i32 = call %i
    %23:u32 = mul %51, 4u
    %52:u32 = add %10, %13
    %53:u32 = add %52, %16
    %54:u32 = add %53, %23
    %55:u32 = div %54, 16u
    %56:ptr<uniform, vec4<u32>, read> = access %a, %55
    %57:u32 = mod %54, 16u
    %58:u32 = div %57, 4u
    %59:u32 = load_vector_element %56, %58
    %60:f32 = bitcast %59
    %l_a_i_a_i_m_i_i:f32 = let %60
    ret
  }
}
%41 = func(%start_byte_offset:u32):mat2x3<f32> {
  $B4: {
    %63:u32 = div %start_byte_offset, 16u
    %64:ptr<uniform, vec4<u32>, read> = access %a, %63
    %65:vec4<u32> = load %64
    %66:vec3<u32> = swizzle %65, xyz
    %67:vec3<f32> = bitcast %66
    %68:u32 = add 16u, %start_byte_offset
    %69:u32 = div %68, 16u
    %70:ptr<uniform, vec4<u32>, read> = access %a, %69
    %71:vec4<u32> = load %70
    %72:vec3<u32> = swizzle %71, xyz
    %73:vec3<f32> = bitcast %72
    %74:mat2x3<f32> = construct %67, %73
    ret %74
  }
}
%37 = func(%start_byte_offset_1:u32):Inner {  # %start_byte_offset_1: 'start_byte_offset'
  $B5: {
    %76:mat2x3<f32> = call %41, %start_byte_offset_1
    %77:Inner = construct %76
    ret %77
  }
}
%31 = func(%start_byte_offset_2:u32):array<Inner, 4> {  # %start_byte_offset_2: 'start_byte_offset'
  $B6: {
    %a_1:ptr<function, array<Inner, 4>, read_write> = var, array<Inner, 4>(Inner(mat2x3<f32>(vec3<f32>(0.0f))))  # %a_1: 'a'
    loop [i: $B7, b: $B8, c: $B9] {  # loop_1
      $B7: {  # initializer
        next_iteration 0u  # -> $B8
      }
      $B8 (%idx:u32): {  # body
        %81:bool = gte %idx, 4u
        if %81 [t: $B10] {  # if_1
          $B10: {  # true
            exit_loop  # loop_1
          }
        }
        %82:u32 = mul %idx, 64u
        %83:u32 = add %start_byte_offset_2, %82
        %84:ptr<function, Inner, read_write> = access %a_1, %idx
        %85:Inner = call %37, %83
        store %84, %85
        continue  # -> $B9
      }
      $B9: {  # continuing
        %86:u32 = add %idx, 1u
        next_iteration %86  # -> $B8
      }
    }
    %87:array<Inner, 4> = load %a_1
    ret %87
  }
}
%25 = func(%start_byte_offset_3:u32):Outer {  # %start_byte_offset_3: 'start_byte_offset'
  $B11: {
    %89:array<Inner, 4> = call %31, %start_byte_offset_3
    %90:Outer = construct %89
    ret %90
  }
}
%18 = func(%start_byte_offset_4:u32):array<Outer, 4> {  # %start_byte_offset_4: 'start_byte_offset'
  $B12: {
    %a_2:ptr<function, array<Outer, 4>, read_write> = var, array<Outer, 4>(Outer(array<Inner, 4>(Inner(mat2x3<f32>(vec3<f32>(0.0f))))))  # %a_2: 'a'
    loop [i: $B13, b: $B14, c: $B15] {  # loop_2
      $B13: {  # initializer
        next_iteration 0u  # -> $B14
      }
      $B14 (%idx_1:u32): {  # body
        %94:bool = gte %idx_1, 4u
        if %94 [t: $B16] {  # if_2
          $B16: {  # true
            exit_loop  # loop_2
          }
        }
        %95:u32 = mul %idx_1, 256u
        %96:u32 = add %start_byte_offset_4, %95
        %97:ptr<function, Outer, read_write> = access %a_2, %idx_1
        %98:Outer = call %25, %96
        store %97, %98
        continue  # -> $B15
      }
      $B15: {  # continuing
        %99:u32 = add %idx_1, 1u
        next_iteration %99  # -> $B14
      }
    }
    %100:array<Outer, 4> = load %a_2
    ret %100
  }
}

