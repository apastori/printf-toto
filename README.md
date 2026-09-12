# printf-toto

POSIX-style `printf` utility: format a string and arguments, write the result
to stdout. Output chunks use raw `write(2)`.

**POSIX core** scope: `%s` `%c` `%d` `%i` `%u` `%o` `%x` `%X` `%%`, backslash
escapes in the format, flags/width/precision (including `*`), and format reuse
when arguments remain. Floating conversions, `%b`, and Bash `%q` are **not**
supported.

## Build

```sh
make
make debug
make test
make clean
# optional (Linux/macOS):
# make install
```

Artefacts:

- `build/printf-toto`
- `build/printf-toto-debug`
- `build/tests/test_core`

**Linux:**

```sh
make clean && make
```

**Windows (MSYS2 UCRT64):**

```sh
pacman -S make mingw-w64-ucrt-x86_64-gcc   # once
make clean && make
```

Git Bash needs UCRT `bin` early on `PATH`. The native
Windows `.exe` runs in UCRT64, Git Bash, cmd, and PowerShell.

## Usage

```text
printf-toto FORMAT [ARGUMENT...]
```

Missing `FORMAT` is an error. `--help` / `--h` and `--version` / `--v` are
recognised only when they are the sole operand (no single-dash `-h` / `-v`).

```sh
./build/printf-toto 'Hello\n'
./build/printf-toto 'Hello, %s!\n' World
./build/printf-toto '%d\n' 42
./build/printf-toto '0x%x\n' 255
./build/printf-toto '100%%\n'
./build/printf-toto '%s\n' a b c
./build/printf-toto %s --help
```

`printf-toto %s --help` prints `--help` (meta-flags only when sole operand).

## Exit codes

| Code | Meaning |
|------|---------|
| `0` | Success; also `--help` / `--version` |
| `1` | Missing format, conversion/format error (final status), write/setup failure |
| `130` | Ctrl+C / `SIGINT` |

Errno diagnostics on stderr: `printf-toto: <context>: <reason>`.

Broken pipe on `write` exits `1` (Linux ignores `SIGPIPE` first so `write`
returns `EPIPE`).

## Layout

```text
LICENSE.txt
c_version.txt
Makefile
README.md
include/printf_toto.h
include/printf_toto_*.h
src/main.c
src/printf_toto_{emit,format,write,cli}.c
tests/test_runner.c
tests/test_*.c
build/          # objects/binaries; dirs via .gitkeep
```
