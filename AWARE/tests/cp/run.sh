export PTX_SIM_KERNELFILE=_0.ptx
export PTX_SIM_USE_PTX_FILE=1
export DELAYED_REC_INFO=delayed_rec_CP.config
./EA_Ropa --qatest --odump --atomic --clpath=./clothphysics_data/ropa.cl --data=./clothphysics_data/ --input=tshirt_many.bin

