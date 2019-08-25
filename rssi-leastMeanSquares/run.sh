
namelist=(0 4 8 12)
cd ../analysis-optimizationResults-lms
. clear.sh
cd ../rssi-leastMeanSquares/
for((i=0;i<4;i++))do
	sigma=${namelist[i]}
	echo sigma : $sigma
	sed -i "s/sigma_TiAj_dBm 0/sigma_TiAj_dBm $sigma/g"  Data_SigmaAndThreshold.txt
	cd build
	rm log*
	make
	./alice
	cp log_* ../../analysis-optimizationResults-lms/data_${sigma}sigma/
	cd ..
	sed -i "s/sigma_TiAj_dBm $sigma/sigma_TiAj_dBm 0/g"  Data_SigmaAndThreshold.txt
done

cd ../analysis-optimizationResults-lms-weighted
. clear.sh
cd ../rssi-leastMeanSquares/

P_Th=-20
for((i=0;i<4;i++))do
	sigma=${namelist[i]}
	echo sigma : $sigma
	sed -i "s/sigma_TiAj_dBm 0/sigma_TiAj_dBm $sigma/g"  Data_SigmaAndThreshold.txt
	sed -i "s/Threshold_dBm -300/Threshold_dBm $P_Th/g"  Data_SigmaAndThreshold.txt
	cd build
	rm log*
	make
	./alice
	cp log_* ../../analysis-optimizationResults-lms-weighted/data_${sigma}sigma/
	cd ..
	sed -i "s/sigma_TiAj_dBm $sigma/sigma_TiAj_dBm 0/g"  Data_SigmaAndThreshold.txt
	sed -i "s/Threshold_dBm $P_Th/Threshold_dBm -300/g"  Data_SigmaAndThreshold.txt
done

