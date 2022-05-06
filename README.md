# cuteFPGA

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![Windows Build](https://github.com/JerryAZR/cuteFPGA/actions/workflows/windows-build.yml/badge.svg)](https://github.com/JerryAZR/cuteFPGA/actions/workflows/windows-build.yml)
[![Linux Build](https://github.com/JerryAZR/cuteFPGA/actions/workflows/linux-build.yml/badge.svg)](https://github.com/JerryAZR/cuteFPGA/actions/workflows/linux-build.yml)

A non-official GUI wrapper of the [YosysHQ/oss-cad-suite](https://github.com/YosysHQ/oss-cad-suite-build),
built with Qt6 framework (hence the name **cute**FPGA)

This project is currently a work in progress and only supports a very small
subset of devices. I have only tested it on several [webFPGA](https://webfpga.io/) boards.

The **oss-cad-suite** is not included in the executable because it is very large
(several GBs after decompressions). That being said, you can download the
entire CAD suite using this application. The downloaded files (and everything else)
are stored in `$HOME/.cuteFPGA/`
