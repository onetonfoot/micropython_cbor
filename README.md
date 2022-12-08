# Micropython CBOR

A native c module for CBOR encoding

```py
import ucbor

d = {
	"x" : 10,
	"y" : 11,
}

bs = ucbor.dumps(d)
ucbor.loads(bs)
```

# Building

```
pip install pyelftools
git clone https://github.com/micropython/micropython.git /opt/micropython
ARCH=armv6 make
```

# References

* https://github.com/micropython/micropython/issues/5641
* https://github.com/jeremyherbert/micropython/blob/3a41f2e935fb33f070b77ad77fc3bd2488d4d922/examples/natmod/ucbor/ucbor.c
* https://github.com/micropython/micropython/issues/5597
* https://github.com/micropython/micropython/issues/8009
* https://forums.raspberrypi.com/viewtopic.php?t=305744#p1880732
* https://stackoverflow.com/questions/29344048/undefined-reference-to-memcpy-in-arm-none-eabi-link-chain
* https://clc-wiki.net/wiki/memcpy#Implementation
* https://www.openembedded.org/pipermail/openembedded-devel/2011-April/077133.html
* https://www.geeksforgeeks.org/write-memcpy/


This is built on top of [tinycbor v0.6](https://github.com/intel/tinycbor/tree/v0.6.0), the original source code was modified to remove large switch statements to `undefined reference to __gnu_thumb1_case_uqi` when compiling for arm.
