# LC-3 Emulator and Assembler

A very basic implementation of the LC-3 fictional computer. This CPU has lived rent-free in my head since college when I had to write some projects for it. I'm re-learning C for some personal projects and built this emulator over the course of two days as practice.

I'm not if anyone uses it still (educational or otherwise), but fun nevertheless!

## Usage

```
Usage: lc3 <command> [<args>]

Subcommands:
   exec <file>.bin : Execute machine code.
   asm <file>.s    : Assemble a file into machine code.
   run <file>.s    : Assemble a file and execute it.
```

## Developer Setup

I've only tested this on a Macbook with the provided Makefile. No guarantees are made for any other platforms.

Requirements:

* A `gcc` compiler
* `make`

Building and Running: `make`.


# References

* [Slides from UTexas](https://www.cs.utexas.edu/~fussell/courses/cs310h/lectures/Lecture_10-310h.pdf)
* [Blog](https://medium.com/@saehwanpark/diving-deeper-into-lc-3-from-opcodes-to-machine-code-4637cf00c878)
 (not affiliated with author)
