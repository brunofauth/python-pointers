import subprocess as sp
import memory
import struct
import fire


COBAIA = "./cobaia"
kwargs = {
    #"text": True,
    "shell": True,
    "stdin": sp.PIPE,
    "stdout": sp.PIPE,
    "stderr": sp.PIPE
}


def test_read():
#    with sp.Popen(COBAIA, **kwargs) as process:
#        procmem = memory.Process(process.pid)
#        addr, val = process.stdout.readline().split()
#        print("after er")
#        buff = process.read(int(addr, 16), 4)
#        print("antes do assert")
#        assert struct.unpack("i", buff) == int(val, 16)
#        print("deposit do assert")
    process = sp.Popen(COBAIA, **kwargs)
    procmem = memory.Process(process.pid)
    print(process.stdout.readline())
    process.terminate()


def test_write():
    with sp.Popen(COBAIA, **kwargs) as process:
        procmem = memory.Process(process.pid)
        addr1, val1 = process.stdout.readline().split()
        procmem.write(int(addr1, 16), struct.pack("i", 3))
        process.stdin.write("\n")
        addr2, val2 = process.stdout.readline().split()
        assert addr1 == addr2 and val1 != val2 == 3


if __name__ == "__main__":
    fire.Fire({"read": test_read, "write": test_write})
