export PTX_SIM_KERNELFILE=hashtable.O2.cu.ptx
export PTX_SIM_USE_PTX_FILE=1
export DELAYED_REC_INFO=delayed_rec_HT.config
./ht -atomic --numBlocks=240 --hashEntries=81920
