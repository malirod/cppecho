#!/bin/bash

IWYU_WARN_LOG=$(mktemp -t iwyu_warn.XXXXXX)
IWYU_ALL_LOG=$(mktemp -t iwyu_all.XXXXXX)
trap "rm -f $IWYU_WARN_LOG $IWYU_ALL_LOG" EXIT

# Reads path to iwyu_tool.py from cmd line args (param 1)
# Param 2 holds path to the project root
IWYU_TOOL_PATH=$1
PROJECT_ROOT=$2
#python -u $IWYU_TOOL_PATH --j=$(nproc) -p . -- --mapping_file=$PROJECT_ROOT/tools/iwyu/project.imp --check_also=*/src/*.h 2>&1 | tee $IWYU_ALL_LOG
python -u $IWYU_TOOL_PATH --j=$(nproc) -p . -- --mapping_file=$PROJECT_ROOT/tools/iwyu/project.imp 2>&1 | tee $IWYU_ALL_LOG

echo "IWYU: checking output"
cat $IWYU_ALL_LOG | awk -f $PROJECT_ROOT/tools/iwyu/iwyu.awk | tee $IWYU_WARN_LOG

if [ -s "$IWYU_WARN_LOG" ]; then
  echo "IWYU: Warings were generated"
  exit 1
fi

echo "IWYU: no warnings"
exit 0
