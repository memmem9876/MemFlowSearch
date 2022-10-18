# MemFlowSearch
# QEMU Windows Setup
sudo apt install -y qemu qemu-kvm libvirt-daemon libvirt-clients bridge-utils virt-manager  
sudo poweroff --reboot  
virt-manager  
Windows install  
...  


# Setup
#Ubuntu 22.04.1

### clang16 Install  
#https://apt.llvm.org/  
#deb http://apt.llvm.org/jammy/ llvm-toolchain-jammy main  
#sudo wget -O - http://llvm.org/apt/llvm-snapshot.gpg.key|sudo apt-key add -  
#sudo add-apt-repository "deb-src http://apt.llvm.org/jammy/ llvm-toolchain-jammy main"  
wget -qO - http://llvm.org/apt/llvm-snapshot.gpg.key | gpg --dearmor | sudo tee /usr/share/keyrings/llvm-snapshot.gpg > /dev/null  
echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/llvm-snapshot.gpg] http://apt.llvm.org/jammy/ llvm-toolchain-jammy main" | sudo tee /etc/apt/sources.list.d/llvm-snapshot.list  

sudo apt update  
sudo apt install -y clang  
sudo apt install -y libc++-16-dev  
cd /usr/lib/x86_64-linux-gnu  
sudo ln -s  libc++abi.so.1 libc++abi.so  
cd  

### git,curl Install  
sudo apt install -y git  
sudo apt install -y curl  

### Rust Install    
curl https://sh.rustup.rs -sSf | sh  
#1  
source "$HOME/.cargo/env"  

### memflowup 
mkdir memwork  
cd memwork  
git clone https://github.com/memflow/memflowup.git  
cd memflowup  
#curl --proto '=https' --tlsv1.2 -sSf https://sh.memflow.io | sh  
cargo install memflowup --force --version "=0.1.0-beta11"  
memflowup install memflow-win32 memflow-qemu --system --dev --from-source  
#sudo apt install -y dkms  
#memflowup install memflow-kvm --system --dev --from-source  
#sudo modprobe memflow  
cd ..  

### memflow_ffi build  
git clone https://github.com/memflow/memflow.git  
cd memflow  
cargo build --release --all-features --workspace  
cd ..  

### MemFlowSearch  
sudo apt install -y make  
git clone https://github.com/games24-hp1234/MemFlowSearch.git  
cp ./memflow/target/release/libmemflow_ffi.a ./MemFlowSearch/lib/libmemflow_ffi.a  
cp ./memflow/memflow-ffi/memflow.hpp ./MemFlowSearch/include/memflow.hpp  
cd MemFlowSearch  
make  
./main.out  

# vscode Setup  
### vscode server
sudo wget -O- https://aka.ms/install-vscode-server/setup.sh | sh
ip -4 a
#sudo apt install -y gnome-keyring
dbus-run-session -- sh -c "(echo 'yoursecrethere' | gnome-keyring-daemon --unlock) && code-server serve-local --host 0.0.0.0"

#Web UI available at http://192.168.xx.xx:8000/?tkn=8xxxxx-xxxx-xxxx-...  
#http://192.168.xx.xx:8000/?tkn=xxxxx.....


### vscode root  
#echo "sudo /usr/bin/gdb \"\$@\"" >> gdb  
#sudo chmod 777 gdb  

#sudo nano /etc/sudoers  
#c ALL=(ALL) NOPASSWD:/usr/bin/gdb  
echo "c ALL=(ALL) NOPASSWD:/usr/bin/gdb" | sudo tee -a /etc/sudoers  
sudo chmod 0440 /etc/sudoers  
