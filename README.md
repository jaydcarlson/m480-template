# Nuvoton M480 Makefile-based template
Fork this repo to start a Nuvoton M480 project. This directory tree contains a recent SDK, a Makefile, and vscode tasks and configurations for debugging using a J-Link debugger.

## Requirements
Makefile-based GCC development requires Make and an ARM GCC to be installed. You can see if you have them properly installed and on your path by running `make` and `arm-none-eabi-gcc` from a PowerShell prompt (obviously if they are installed, you'll get error outputs from them for not providing proper inputs, etc).

This repo is also designed for a VSCode-based workflow, so you'll need that, too.

You can use the [Chocolatey](https://chocolatey.org/install) package manager from an **elevated PowerShell prompt** to get everything else installed:
```
PS > choco install gcc-arm-embedded
PS > choco install make
PS > choco install vscode
PS > choco install vscode-cortex-debug
PS > choco install vscode-cpptools
```

If you don't want to use Chocolatey, you can install these tools manually:
- [ARM GCC compiler](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads)
- GNU Make [Windows binaries](http://gnuwin32.sourceforge.net/packages/make.htm)
- [Visual Studio Code](https://code.visualstudio.com/)
- [C/C++](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools)
- [Cortex-Debug](https://marketplace.visualstudio.com/items?itemName=marus25.cortex-debug)

Chocolatey automatically adds GCC and Make to your path; if you install the packages manually, you'll have to edit your path to ensure all the tools are available from a command prompt.

You'll need to manually add J-Link binaries to your path, though.

The easiest way to get to the path edit dialogue is to press Start and begin typing "Edit environment variables..." until the appropriate option appears.

## Changes you need to make
- change the TARGET in the Makefile to the name of your project. This name needs to be set in `launch.json` wherever "Target" appears.
- In `tasks.json` change "make -j12" to the number of threads on your system (default is 12)

## Experimental NuLink support
Nuvoton dev boards have a NuLink debugger on board. This OpenOCD-based debugger is very slow, requires a Nuvoton-specific build of OpenOCD (part of [NuEclipse](https://www.nuvoton.com/tool-and-software/ide-and-compiler/)), and doesn't currently work with Cortex-Debug, but it does work with the vanilla cppdbg backend in vscode. 

To use this, you'll have to manually invoke the NuLinkOpenOCD task (Ctrl-Shift-P, then search for "Run Task" and select the NuLinkOpenOCD task) before running the NuLink launch configuration. Note that OpenOCD exits when the GDB connection is closed, so you'll have to repeat this before each debug launch.