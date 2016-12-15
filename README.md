# WIP

```bash
git clone https://github.com/tangrams/tangram-py.git
cd tangram-py/tangram
./configure
```


## Research

```bash
swig -python example.i

gcc -c example.c example_wrap.c -I/System/Library/Frameworks/Python.framework/Versions/2.6/include/python2.6/

ld -bundle -flat_namespace -macosx_version_min 10.12 -undefined suppress -o _example.so *.o
```

### Resources

- http://swig.org/tutorial.html
- http://www.expobrain.net/2011/01/23/swig-tutorial-for-mac-os-x/

### Examples

- https://github.com/johnglover/pyof
- https://github.com/lygstate/tangram-es/tree/master/core/api