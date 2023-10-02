bin/champsim_stream_53 --warmup_instructions 25000000 --simulation_instructions 25000000 ~/Downloads/CS683/Tracefiles/602.gcc_s-1850B.champsimtrace.xz > ./logs/602_stream_53.txt
bin/champsim_stream_53 --warmup_instructions 25000000 --simulation_instructions 25000000 ~/Downloads/CS683/Tracefiles/603.bwaves_s-1740B.champsimtrace.xz > ./logs/603_stream_53.txt
bin/champsim_stream_53 --warmup_instructions 25000000 --simulation_instructions 25000000 ~/Downloads/CS683/Tracefiles/619.lbm_s-2677B.champsimtrace.xz > ./logs/619_stream_53.txt
bin/champsim_stream_53 --warmup_instructions 25000000 --simulation_instructions 25000000 ~/Downloads/CS683/Tracefiles/bc-0.trace.gz > ./logs/bc-0_stream_53.txt
bin/champsim_stream_53 --warmup_instructions 25000000 --simulation_instructions 25000000 ~/Downloads/CS683/Tracefiles/sssp-3.trace.gz > ./logs/sssp-3_stream_53.txt

echo "Stream 53 done"

bin/champsim_ip_stride_throttling --warmup_instructions 25000000 --simulation_instructions 25000000 ~/Downloads/CS683/Tracefiles/602.gcc_s-1850B.champsimtrace.xz > ./logs/602_ip_stride_throttling.txt
bin/champsim_ip_stride_throttling --warmup_instructions 25000000 --simulation_instructions 25000000 ~/Downloads/CS683/Tracefiles/603.bwaves_s-1740B.champsimtrace.xz > ./logs/603_ip_stride_throttling.txt
bin/champsim_ip_stride_throttling --warmup_instructions 25000000 --simulation_instructions 25000000 ~/Downloads/CS683/Tracefiles/619.lbm_s-2677B.champsimtrace.xz > ./logs/619_ip_stride_throttling.txt
bin/champsim_ip_stride_throttling --warmup_instructions 25000000 --simulation_instructions 25000000 ~/Downloads/CS683/Tracefiles/bc-0.trace.gz > ./logs/bc-0_ip_stride_throttling.txt
bin/champsim_ip_stride_throttling --warmup_instructions 25000000 --simulation_instructions 25000000 ~/Downloads/CS683/Tracefiles/sssp-3.trace.gz > ./logs/sssp-3_ip_stride_throttling.txt

echo "IP Stride Throttling Done"

bin/champsim_stream_throttling --warmup_instructions 25000000 --simulation_instructions 25000000 ~/Downloads/CS683/Tracefiles/602.gcc_s-1850B.champsimtrace.xz > ./logs/602_stream_throttling.txt
bin/champsim_stream_throttling --warmup_instructions 25000000 --simulation_instructions 25000000 ~/Downloads/CS683/Tracefiles/603.bwaves_s-1740B.champsimtrace.xz > ./logs/603_stream_throttling.txt
bin/champsim_stream_throttling --warmup_instructions 25000000 --simulation_instructions 25000000 ~/Downloads/CS683/Tracefiles/619.lbm_s-2677B.champsimtrace.xz > ./logs/619_stream_throttling.txt
bin/champsim_stream_throttling --warmup_instructions 25000000 --simulation_instructions 25000000 ~/Downloads/CS683/Tracefiles/bc-0.trace.gz > ./logs/bc-0_stream_throttling.txt
bin/champsim_stream_throttling --warmup_instructions 25000000 --simulation_instructions 25000000 ~/Downloads/CS683/Tracefiles/sssp-3.trace.gz > ./logs/sssp-3_stream_throttling.txt

echo "Stream Throttling Done"