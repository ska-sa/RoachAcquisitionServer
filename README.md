# Roach Acquisition Server

## Clone Repo
`git clone --recurse-submodules git@github.com:ska-sa/RoachAcquisitionServer.git`

## Checkout
If making changes to the SpectrometerDataStream repo (submodule located in AVNDataTypes), you must checkout the `scam_integration` branch.

## Commit and Push
When committing and pushing, any changes to the SpectrometerDataStream repo must be committed first, so that the link to the submodule is preserved. Push the submodule, and then the main repo.

## Build Image
`docker build --no-cache -t roach2 .`

## Run Container
```
docker run -dt --name roach2_environment --network host --rm \
-v $HOME/Data/RoachAcquisition:/data \
-v $HOME/AVNSoftware/RoachLaunchers:/workspace/RoachLaunchers \
-v $HOME/AVNSoftware/RoachAcquisitionServer/NoiseDiode:/etc/NoiseDiode \
-v /etc/ObservatoryInfo.ini:/etc/ObservatoryInfo.ini \
roach2 /workspace/RoachAcquisitionServer/bin/RoachAcquisitionServer \
--local-interface=10.0.3.1 \
--roach-address=10.0.3.2 \
--roach-ppc-address=10.0.2.43 \
--roach-gateware-dir=/workspace/RoachLaunchers \
--station-controller-address=172.16.16.134 \
--station-controller-port=40001 \
--recording-dir=/data
```

## Site deployment
Add the servers (Reber) ssh key to your GitHub account. Git pull as normal.
After pulling, submodules may need to be sync'd: `git submodule update --init --recursive`
If desired, save a copy of the current image: `docker tag roach2:latest roach2:20260501`
Build the image: `docker --no-cache build -t roach2 .`
Run by executing `./startWBS.sh` in the home directory

## Testing a branch on site
Modify the Dockerfile in `/home/avnuser/AVNSoftware/RoachAcquisitionServer` add the followiong lines after the `WORKDIR /workspace/RoachAcquisitionServer`
```
RUN git checkout <branch-name>
RUN git submodule update --init --recursive
```
