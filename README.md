## xv6-riscv (JayWei's fork)

This is my personal fork of **MIT xv6-riscv**, used for learning operating system (OS) development.

- **System**：WSL2 (Ubuntu 20.04 LTS).
- **Goal**: Run and debug xv6-riscv locally using QEMU.

---

#### 1. Environment Setup

##### 1.1. Install QEMU build dependencies

~~~shell
sudo apt install -y build-essential libglib2.0-dev libpixman-1-dev git ninja-build pkg-config
~~~

Notes:

- This is the minimal dependency set required to build QEMU from source
- If you use the system-provided QEMU, you may skip the entire QEMU build section

---

#### 2. Build and Install QEMU (7.2.0)

> xv6-riscv recommends using a relatively recent QEMU version. The QEMU version shipped with Ubuntu 20.04 is outdated, so building from source is preferred.

##### 2.1. Clone the QEMU repository

~~~shell
git clone https://github.com/qemu/qemu.git
cd qemu
~~~

##### 2.2. Checkout QEMU v7.2.0

~~~shell
git checkout v7.2.0
~~~

##### 2.3. Configure (build only RISC-V targets)

~~~shell
./configure --target-list=riscv64-softmmu --prefix=/usr/local
~~~

Notes:

- `riscv64-softmmu` is the only target required by xv6-riscv
- Restricting targets significantly reduces build time

##### 2.4. Build and install

~~~shell
make -j$(nproc)
sudo make install
~~~

##### 2.5. Verify installation

~~~shell
qemu-system-riscv64 --version
~~~

If the reported version is **≥ 7.2.0**, the installation is successful.

---

#### 3. Run xv6-riscv

Return to the xv6-riscv source directory:

~~~shell
cd ~/xv6-riscv
make qemu
~~~

If QEMU starts successfully and you enter the xv6 shell, the environment is correctly set up.

