# Formatted Binary File Reader | c-bin-format-reader
> Created by Nicholas Ramsay

A utility for reading binary files with a custom format string.

## Usage
```sh
$ main "5%d4%s" blob.bin
4hello
```

### Format tokens
Follow the `%` with `d`, `f`, `c`, `s` to specify the output type.

* `%d` - a 4 byte integer
* `%f` - a 4 byte float
* `%c` - a 1 byte character
* `%s` - read chars until a null terminator is read.

To adjust the byte count, follow the `%` with the byte count. E.g. `%8d` is equivalent to an 8 byte long int. 

To repeat a read simply prepend the repeat count. E.g `5%8d` will read 5 long ints.

### Repeat token blocks
To repeat blocks use `[` and `]`. E.g. `[%c%d]` will read a character then a 4 byte integer indefinitely. To limit the repeats simply prepend a number. E.g. `5[%c%d]` will read a character then a 4 byte integer 5 times.

## Roadmap
- [x] Single type specifier parsing w/ specified byte count
- [ ] Repeat single type specifier parsing w/ specified byte count
- [ ] Repeat block parsing
- [ ] `-r` repeat flag
- [ ] `-w` warning flag (e.g. warning for nested infinite repeat blocks)