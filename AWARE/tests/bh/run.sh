export PTX_SIM_KERNELFILE=BarnesHutCUDA.O2.cu.ptx
export PTX_SIM_USE_PTX_FILE=1
export DELAYED_REC_INFO=delayed_rec_BH.config 
#export PTX_SIM_DEBUG=6
#export PTX_SIM_DEBUG_CYCLE=42780
./bh 3000 1 0
