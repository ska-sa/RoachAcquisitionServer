cat startWBS.sh
#!/bin/sh
echo "Start the Wideband Specrometer"
echo "Watch the error messages - specifically if 10 GbE link is up"
cd /home/avnuser/AVNSoftware/RoachLaunchers
python LaunchWBSpectrometer.py
sleep 5s
echo "Start the acquisition server in new window"
echo "Watch the output for error messages"
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
