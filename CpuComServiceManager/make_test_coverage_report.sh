#!/bin/bash
###---Version-tag:1.0

[ -n "$ANDROID_BUILD_TOP" ] || \
    { echo "ERROR: \$ANDROID_BUILD_TOP variable is not set" >&2; exit 1; }
android_root="$ANDROID_BUILD_TOP"

test_coverage_report_generator="${android_root}/vendor/melco/efw/devtools-config/\
unit-test-common-java/test_coverage_report_generator.sh"

################################################################################
# These variables are set per unit test target, adjust if needed.
################################################################################
src_dir="${android_root}/vendor/melco/efw/CpuComm/CpuComServiceManager"

test_name="CpuComServiceManager"
################################################################################

${test_coverage_report_generator} \
    --src-dir "${src_dir}" \
    --test-name "${test_name}" \
    "$@"
