
#PERF_FLAGS="-g -F 10000  -e cycles"
PERF_FLAGS="-g -e cycles"
PERF_FLAGS="${PERF_FLAGS},instructions"
PERF_FLAGS="${PERF_FLAGS},cache-references"
PERF_FLAGS="${PERF_FLAGS},cache-misses"
PERF_FLAGS="${PERF_FLAGS},LLC-loads"
PERF_FLAGS="${PERF_FLAGS},LLC-load-misses"
PERF_FLAGS="${PERF_FLAGS},LLC-stores"
PERF_FLAGS="${PERF_FLAGS},LLC-store-misses"
PERF_FLAGS="${PERF_FLAGS},LLC-prefetch-misses"
PERF_FLAGS="${PERF_FLAGS},dTLB-load-misses"
PERF_FLAGS="${PERF_FLAGS},dTLB-store-misses"

perf record ${PERF_FLAGS} ./main --opt=a --gb=5
perf record ${PERF_FLAGS} ./main --opt=b --gb=5

#perf stat ${PERF_FLAGS} ./main --opt=a --gb=5
#perf stat ${PERF_FLAGS} ./main --opt=b --gb=5

