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

Failed to generate: :40:24 error: binary: %26 is not in scope
    %25:u32 = add %24, %26
                       ^^^

:24:3 note: in block
  $B3: {
  ^^^

:60:5 note: %26 declared here
    %26:u32 = mul %48, 4u
    ^^^^^^^

:45:24 error: binary: %26 is not in scope
    %32:u32 = add %31, %26
                       ^^^

:24:3 note: in block
  $B3: {
  ^^^

:60:5 note: %26 declared here
    %26:u32 = mul %48, 4u
    ^^^^^^^

:60:5 error: binary: no matching overload for 'operator * (i32, u32)'

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

    %26:u32 = mul %48, 4u
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
    %20:Outer = call %21, %10
    %l_a_i:Outer = let %20
    %23:u32 = add %10, %13
    %24:u32 = add %23, %16
    %25:u32 = add %24, %26
    %27:array<Inner, 4> = call %28, %25
    %l_a_i_a:array<Inner, 4> = let %27
    %30:u32 = add %10, %13
    %31:u32 = add %30, %16
    %32:u32 = add %31, %26
    %33:Inner = call %34, %32
    %l_a_i_a_i:Inner = let %33
    %36:u32 = add %10, %13
    %37:mat2x3<f32> = call %38, %36
    %l_a_i_a_i_m:mat2x3<f32> = let %37
    %40:u32 = add %10, %13
    %41:u32 = add %40, %16
    %42:u32 = div %41, 16u
    %43:ptr<uniform, vec4<u32>, read> = access %a, %42
    %44:vec4<u32> = load %43
    %45:vec3<u32> = swizzle %44, xyz
    %46:vec3<f32> = bitcast %45
    %l_a_i_a_i_m_i:vec3<f32> = let %46
    %48:i32 = call %i
    %26:u32 = mul %48, 4u
    %49:u32 = add %10, %13
    %50:u32 = add %49, %16
    %51:u32 = add %50, %26
    %52:u32 = div %51, 16u
    %53:ptr<uniform, vec4<u32>, read> = access %a, %52
    %54:u32 = mod %51, 16u
    %55:u32 = div %54, 4u
    %56:u32 = load_vector_element %53, %55
    %57:f32 = bitcast %56
    %l_a_i_a_i_m_i_i:f32 = let %57
    ret
  }
}
%21 = func(%start_byte_offset:u32):Outer {
  $B4: {
    %60:array<Inner, 4> = call %28, %start_byte_offset
    %61:Outer = construct %60
    ret %61
  }
}
%28 = func(%start_byte_offset_1:u32):array<Inner, 4> {  # %start_byte_offset_1: 'start_byte_offset'
  $B5: {
    %a_1:ptr<function, array<Inner, 4>, read_write> = var, array<Inner, 4>(Inner(mat2x3<f32>(vec3<f32>(0.0f))))  # %a_1: 'a'
    loop [i: $B6, b: $B7, c: $B8] {  # loop_1
      $B6: {  # initializer
        next_iteration 0u  # -> $B7
      }
      $B7 (%idx:u32): {  # body
        %65:bool = gte %idx, 4u
        if %65 [t: $B9] {  # if_1
          $B9: {  # true
            exit_loop  # loop_1
          }
        }
        %66:u32 = mul %idx, 64u
        %67:u32 = add %start_byte_offset_1, %66
        %68:ptr<function, Inner, read_write> = access %a_1, %idx
        %69:Inner = call %34, %67
        store %68, %69
        continue  # -> $B8
      }
      $B8: {  # continuing
        %70:u32 = add %idx, 1u
        next_iteration %70  # -> $B7
      }
    }
    %71:array<Inner, 4> = load %a_1
    ret %71
  }
}
%34 = func(%start_byte_offset_2:u32):Inner {  # %start_byte_offset_2: 'start_byte_offset'
  $B10: {
    %73:mat2x3<f32> = call %38, %start_byte_offset_2
    %74:Inner = construct %73
    ret %74
  }
}
%38 = func(%start_byte_offset_3:u32):mat2x3<f32> {  # %start_byte_offset_3: 'start_byte_offset'
  $B11: {
    %76:u32 = div %start_byte_offset_3, 16u
    %77:ptr<uniform, vec4<u32>, read> = access %a, %76
    %78:vec4<u32> = load %77
    %79:vec3<u32> = swizzle %78, xyz
    %80:vec3<f32> = bitcast %79
    %81:u32 = add 16u, %start_byte_offset_3
    %82:u32 = div %81, 16u
    %83:ptr<uniform, vec4<u32>, read> = access %a, %82
    %84:vec4<u32> = load %83
    %85:vec3<u32> = swizzle %84, xyz
    %86:vec3<f32> = bitcast %85
    %87:mat2x3<f32> = construct %80, %86
    ret %87
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
        %91:bool = gte %idx_1, 4u
        if %91 [t: $B16] {  # if_2
          $B16: {  # true
            exit_loop  # loop_2
          }
        }
        %92:u32 = mul %idx_1, 256u
        %93:u32 = add %start_byte_offset_4, %92
        %94:ptr<function, Outer, read_write> = access %a_2, %idx_1
        %95:Outer = call %21, %93
        store %94, %95
        continue  # -> $B15
      }
      $B15: {  # continuing
        %96:u32 = add %idx_1, 1u
        next_iteration %96  # -> $B14
      }
    }
    %97:array<Outer, 4> = load %a_2
    ret %97
  }
}

