from distutils.core import setup, Extension

if __name__ == "__main__":
    setup(
        name="proq",
        version="1.0",
        ext_modules=[
            Extension("memory", ["memmodule.c"])
        ]
    )

# dbg
# import memory,struct;p=memory.Process(1046);struct.unpack("i", p.read(0x7ffc929e6c84, 4)[1])
