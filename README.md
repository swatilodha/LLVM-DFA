# LLVM Data Flow Analysis

The project consists of the following:
- DataFlow Analysis framework [code](dataflow.cpp)
- Reaching Definitions Pass [code](reaching.cpp)
- Available Expressions Pass [code](available.cpp)
- Liveness Analysis Pass [code](liveness.cpp)

---
## Build Shared Objects

Build the project using :
```
make clean
make
```
---
## Run Passes
The command to run the passes is :
```
./run.sh <pass-name>
```
where `pass-name` can be one of three values :
- available
- reaching
- liveness