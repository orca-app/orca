(module $test.wasm
  (type (;0;) (func (param i32) (result i32)))
  (func $start (type 0) (param i32) (result i32)
    (local i32)
    (if (result i32 i32) (local.get 0)
      (then
        (i32.const 42)
        (i32.const 35))
      (else
        (i32.const 54)
        (i32.const 27)))
    local.set 0
    return)
  (memory (;0;) 2)
  (export "memory" (memory 0))
  (export "start" (func $start)))
