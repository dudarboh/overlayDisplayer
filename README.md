# On the local PC:

## 1. Install iLCSoft

```bash
source /afs/desy.de/project/ilcsoft/sw/x86_64_gcc75_ub1804/v02-02-02/init_ilcsoft.sh
```

## 2. Run event display waiting connections from NAF

```bash
glced -trust naf-ilc11
```


# On the NAF PC:

## 1. Setup the environment
```bash
source /cvmfs/ilc.desy.de/key4hep/setup.sh
# Set CED_HOST to the local PC name.
# run hostname on local PC to check the name (e.g. flc42)
export CED_HOST=flc42
```

## 2. Compile Marlin processor as usual
```bash
git clone https://github.com/dudarboh/overlayDisplayer.git
cd overlayerDisplayer
mkdir build && cd build
cmake ..
make install -j20
export MARLIN_DLL=$MARLIN_DLL:$(realpath ../lib/libOverlayDisplayer.so)
```

## 3. Execute
```bash
Marlin ../xml/steer.xml
```



## What does this processor do

Two drawing functions are defined:

### a) drawEventAllHits()

Will loop over all reconstructed hits from all sensible collections. And draw them. Overlay hits in red, others in blue.

### b) drawEventPFOs()

Will loop over all reconstructed particles (PFOs). And draw all reconstructed hits attached to the track and cluster of each PFO. Overlay hits in red, others in blue.
