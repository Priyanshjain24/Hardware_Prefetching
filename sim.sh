bin/champsim_NoPrefetch --warmup_instructions 25000000 --simulation_instructions 25000000 ~/Downloads/CS683/Tracefiles/602.gcc_s-1850B.champsimtrace.xz > ./results/602_NP.txt
bin/champsim_NoPrefetch --warmup_instructions 25000000 --simulation_instructions 25000000 ~/Downloads/CS683/Tracefiles/603.bwaves_s-1740B.champsimtrace.xz > ./results/603_NP.txt
bin/champsim_NoPrefetch --warmup_instructions 25000000 --simulation_instructions 25000000 ~/Downloads/CS683/Tracefiles/619.lbm_s-2677B.champsimtrace.xz > ./results/619_NP.txt
bin/champsim_NoPrefetch --warmup_instructions 25000000 --simulation_instructions 25000000 ~/Downloads/CS683/Tracefiles/bc-0.trace.gz > ./results/bc_NP.txt
bin/champsim_NoPrefetch --warmup_instructions 25000000 --simulation_instructions 25000000 ~/Downloads/CS683/Tracefiles/sssp-3.trace.gz > ./results/sssp_NP.txt

echo "No Prefeth Done"

bin/champsim_IPSTRIDE2 --warmup_instructions 25000000 --simulation_instructions 25000000 ~/Downloads/CS683/Tracefiles/602.gcc_s-1850B.champsimtrace.xz > ./results/602_IP2.txt
bin/champsim_IPSTRIDE2 --warmup_instructions 25000000 --simulation_instructions 25000000 ~/Downloads/CS683/Tracefiles/603.bwaves_s-1740B.champsimtrace.xz > ./results/603_IP2.txt
bin/champsim_IPSTRIDE2 --warmup_instructions 25000000 --simulation_instructions 25000000 ~/Downloads/CS683/Tracefiles/619.lbm_s-2677B.champsimtrace.xz > ./results/619_IP2.txt
bin/champsim_IPSTRIDE2 --warmup_instructions 25000000 --simulation_instructions 25000000 ~/Downloads/CS683/Tracefiles/bc-0.trace.gz > ./results/bc_IP2.txt
bin/champsim_IPSTRIDE2 --warmup_instructions 25000000 --simulation_instructions 25000000 ~/Downloads/CS683/Tracefiles/sssp-3.trace.gz > ./results/sssp_IP2.txt

echo "Stride 2 Done"

bin/champsim_IPSTRIDE3 --warmup_instructions 25000000 --simulation_instructions 25000000 ~/Downloads/CS683/Tracefiles/602.gcc_s-1850B.champsimtrace.xz > ./results/602_IP3.txt
bin/champsim_IPSTRIDE3 --warmup_instructions 25000000 --simulation_instructions 25000000 ~/Downloads/CS683/Tracefiles/603.bwaves_s-1740B.champsimtrace.xz > ./results/603_IP3.txt
bin/champsim_IPSTRIDE3 --warmup_instructions 25000000 --simulation_instructions 25000000 ~/Downloads/CS683/Tracefiles/619.lbm_s-2677B.champsimtrace.xz > ./results/619_IP3.txt
bin/champsim_IPSTRIDE3 --warmup_instructions 25000000 --simulation_instructions 25000000 ~/Downloads/CS683/Tracefiles/bc-0.trace.gz > ./results/bc_IP3.txt
bin/champsim_IPSTRIDE3 --warmup_instructions 25000000 --simulation_instructions 25000000 ~/Downloads/CS683/Tracefiles/sssp-3.trace.gz > ./results/sssp_IP3.txt

echo "Stride 3 Done"

bin/champsim_IPSTRIDE4 --warmup_instructions 25000000 --simulation_instructions 25000000 ~/Downloads/CS683/Tracefiles/602.gcc_s-1850B.champsimtrace.xz > ./results/602_IP4.txt
bin/champsim_IPSTRIDE4 --warmup_instructions 25000000 --simulation_instructions 25000000 ~/Downloads/CS683/Tracefiles/603.bwaves_s-1740B.champsimtrace.xz > ./results/603_IP4.txt
bin/champsim_IPSTRIDE4 --warmup_instructions 25000000 --simulation_instructions 25000000 ~/Downloads/CS683/Tracefiles/619.lbm_s-2677B.champsimtrace.xz > ./results/619_IP4.txt
bin/champsim_IPSTRIDE4 --warmup_instructions 25000000 --simulation_instructions 25000000 ~/Downloads/CS683/Tracefiles/bc-0.trace.gz > ./results/bc_IP4.txt
bin/champsim_IPSTRIDE4 --warmup_instructions 25000000 --simulation_instructions 25000000 ~/Downloads/CS683/Tracefiles/sssp-3.trace.gz > ./results/sssp_IP4.txt

echo "Stride 4 Done"

bin/champsim_IPSTRIDE5 --warmup_instructions 25000000 --simulation_instructions 25000000 ~/Downloads/CS683/Tracefiles/602.gcc_s-1850B.champsimtrace.xz > ./results/602_IP5.txt
bin/champsim_IPSTRIDE5 --warmup_instructions 25000000 --simulation_instructions 25000000 ~/Downloads/CS683/Tracefiles/603.bwaves_s-1740B.champsimtrace.xz > ./results/603_IP5.txt
bin/champsim_IPSTRIDE5 --warmup_instructions 25000000 --simulation_instructions 25000000 ~/Downloads/CS683/Tracefiles/619.lbm_s-2677B.champsimtrace.xz > ./results/619_IP5.txt
bin/champsim_IPSTRIDE5 --warmup_instructions 25000000 --simulation_instructions 25000000 ~/Downloads/CS683/Tracefiles/bc-0.trace.gz > ./results/bc_IP5.txt
bin/champsim_IPSTRIDE5 --warmup_instructions 25000000 --simulation_instructions 25000000 ~/Downloads/CS683/Tracefiles/sssp-3.trace.gz > ./results/sssp_IP5.txt

echo "Stride 5 Done"

bin/champsim_StreamPrefetcher23 --warmup_instructions 25000000 --simulation_instructions 25000000 ~/Downloads/CS683/Tracefiles/602.gcc_s-1850B.champsimtrace.xz > ./results/602_ST23.txt
bin/champsim_StreamPrefetcher23 --warmup_instructions 25000000 --simulation_instructions 25000000 ~/Downloads/CS683/Tracefiles/603.bwaves_s-1740B.champsimtrace.xz > ./results/603_ST23.txt
bin/champsim_StreamPrefetcher23 --warmup_instructions 25000000 --simulation_instructions 25000000 ~/Downloads/CS683/Tracefiles/619.lbm_s-2677B.champsimtrace.xz > ./results/619_ST23.txt
bin/champsim_StreamPrefetcher23 --warmup_instructions 25000000 --simulation_instructions 25000000 ~/Downloads/CS683/Tracefiles/bc-0.trace.gz > ./results/bc_ST23.txt
bin/champsim_StreamPrefetcher23 --warmup_instructions 25000000 --simulation_instructions 25000000 ~/Downloads/CS683/Tracefiles/sssp-3.trace.gz > ./results/sssp_ST23.txt

echo "Degree 2 Distance 3 Done"

bin/champsim_StreamPrefetcher24 --warmup_instructions 25000000 --simulation_instructions 25000000 ~/Downloads/CS683/Tracefiles/602.gcc_s-1850B.champsimtrace.xz > ./results/602_ST24.txt
bin/champsim_StreamPrefetcher24 --warmup_instructions 25000000 --simulation_instructions 25000000 ~/Downloads/CS683/Tracefiles/603.bwaves_s-1740B.champsimtrace.xz > ./results/603_ST24.txt
bin/champsim_StreamPrefetcher24 --warmup_instructions 25000000 --simulation_instructions 25000000 ~/Downloads/CS683/Tracefiles/619.lbm_s-2677B.champsimtrace.xz > ./results/619_ST24.txt
bin/champsim_StreamPrefetcher24 --warmup_instructions 25000000 --simulation_instructions 25000000 ~/Downloads/CS683/Tracefiles/bc-0.trace.gz > ./results/bc_ST24.txt
bin/champsim_StreamPrefetcher24 --warmup_instructions 25000000 --simulation_instructions 25000000 ~/Downloads/CS683/Tracefiles/sssp-3.trace.gz > ./results/sssp_ST24.txt

echo "Degree 2 Distance 4 Done"

bin/champsim_StreamPrefetcher33 --warmup_instructions 25000000 --simulation_instructions 25000000 ~/Downloads/CS683/Tracefiles/602.gcc_s-1850B.champsimtrace.xz > ./results/602_ST33.txt
bin/champsim_StreamPrefetcher33 --warmup_instructions 25000000 --simulation_instructions 25000000 ~/Downloads/CS683/Tracefiles/603.bwaves_s-1740B.champsimtrace.xz > ./results/603_ST33.txt
bin/champsim_StreamPrefetcher33 --warmup_instructions 25000000 --simulation_instructions 25000000 ~/Downloads/CS683/Tracefiles/619.lbm_s-2677B.champsimtrace.xz > ./results/619_ST33.txt
bin/champsim_StreamPrefetcher33 --warmup_instructions 25000000 --simulation_instructions 25000000 ~/Downloads/CS683/Tracefiles/bc-0.trace.gz > ./results/bc_ST33.txt
bin/champsim_StreamPrefetcher33 --warmup_instructions 25000000 --simulation_instructions 25000000 ~/Downloads/CS683/Tracefiles/sssp-3.trace.gz > ./results/sssp_ST33.txt

echo "Degree 3 Distance 3 Done"

bin/champsim_StreamPrefetcher34 --warmup_instructions 25000000 --simulation_instructions 25000000 ~/Downloads/CS683/Tracefiles/602.gcc_s-1850B.champsimtrace.xz > ./results/602_ST34.txt
bin/champsim_StreamPrefetcher34 --warmup_instructions 25000000 --simulation_instructions 25000000 ~/Downloads/CS683/Tracefiles/603.bwaves_s-1740B.champsimtrace.xz > ./results/603_ST34.txt
bin/champsim_StreamPrefetcher34 --warmup_instructions 25000000 --simulation_instructions 25000000 ~/Downloads/CS683/Tracefiles/619.lbm_s-2677B.champsimtrace.xz > ./results/619_ST34.txt
bin/champsim_StreamPrefetcher34 --warmup_instructions 25000000 --simulation_instructions 25000000 ~/Downloads/CS683/Tracefiles/bc-0.trace.gz > ./results/bc_ST34.txt
bin/champsim_StreamPrefetcher34 --warmup_instructions 25000000 --simulation_instructions 25000000 ~/Downloads/CS683/Tracefiles/sssp-3.trace.gz > ./results/sssp_ST34.txt

echo "Degree 3 Distance 4 Done"
