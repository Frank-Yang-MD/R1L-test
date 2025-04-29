#!/bin/bash

[ -n "$ANDROID_BUILD_TOP" ] || \
    { echo "ERROR: \$ANDROID_BUILD_TOP variable is not set" >&2; exit 1; }
android_root="$ANDROID_BUILD_TOP"
test_coverage_report_generator="${android_root}/vendor/melco/efw/devtools-config/\
unit-test-common/test_coverage_report_generator.sh"

################################################################################
# These variables are set per a unit test target, adjust if needed.
################################################################################
tests_out_dir=${android_root}/out/soong/.intermediates/\
vendor/melco/efw/CpuComm/libCpuCom/libcpucom-tests-coverage/linux_glibc_x86_64

tests_bin=${android_root}/out/host/linux-x86/nativetest64/\
libcpucom-tests-coverage/libcpucom-tests-coverage

src_dir=${android_root}/vendor/melco/efw/CpuComm/libCpuCom

test_name="libcpucom"
################################################################################

${test_coverage_report_generator} \
    --tests-out-dir "${tests_out_dir}" \
    --tests-bin "${tests_bin}" \
    --src-dir "${src_dir}" \
    --test-name "${test_name}" \
    "$@"
