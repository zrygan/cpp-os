# Monorepo for Operating Systems Projects

**By**: Stephen Borja, Erin Chua, Zhean Ganituen, and Aaron Go.

## Setup

Simply execute the code below:

```bash
sudo apt update && sudo apt upgrade -y
sudo apt install -y build-essential cmake
```

## Running Projects

**Important.** See the specific requirements/dependencies for each
project on their respective section.

To run any project, use the script `s.sh`:

```bash
chmod +x s.sh
./s.sh r
# then select the project from the menu
```

## Projects List

Below is a list of projects included in this monorepo. If this is not
updated, see [`CMakeLists.txt`](/CMakeLists.txt).

### osmock

GUI-based OS Mockup using [ImGui](https://github.com/ocornut/imgui).

**Setup**

```bash
sudo apt update && sudo apt upgrade -y
sudo apt install -y xorg-dev libgl1-mesa-dev libglu1-mesa-dev libglfw3-dev 
```

### prosched

Process Scheduler.

**Requiremnts**: none.
