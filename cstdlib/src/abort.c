// NOTE(orca): This is a clang intrinsic. I hope it generates a wasm unreachable. I have not verified this.
// TODO(orca): Verify this.
_Noreturn void abort(void)
{
	__builtin_unreachable();
}
