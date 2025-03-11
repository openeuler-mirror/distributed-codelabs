#!/bin/bash
# usage example: bash path_init.sh /xxxxtest/lib/python3.9/site-packages/dist_executor/xxx
# This shell script will change /usr/bin/python3.9 link target to the python3.9 which installed dist_executor.
# This shell script should in directory */mindpandas/de/custom

# match lib/python3.9 , to /xxxxxx/bin/python3.9
path_left=${1%/lib/python3.9*}
PY_VERSION=`python -V 2>&1 | awk '{print$2}'|awk -F '.' '{print $1"."$2 }'`

if [ "$PY_VERSION" != "3.9" ];then
  echo "please use python3.9" && exit 1
fi

SCRIPT_FILE=$(readlink -f $0)
CUSTOM_DIR=$(dirname $SCRIPT_FILE)
MODULES_DIR=$(dirname $CUSTOM_DIR )
yamlList=$(find $MODULES_DIR -path "*/service.yaml")
for yamlPath in ${yamlList[@]}
do
    yamlDir=${yamlPath%%"/service.yaml"}
    subDirs=$(ls -d $yamlDir/*/  2> /dev/null )
    for subDir in ${subDirs[@]}
    do
        if ls ${subDir}*.py >/dev/null 2>&1; then
            realCodePath=$subDir
            cp ${MODULES_DIR}/resource/local-repo/sdk/yrlib/python3.9/yrlib_handler.py $realCodePath
            codePathDoubleSlash=${realCodePath//"/"/"\\/"}
            sed -i 's/codePath:.*/codePath: '"$codePathDoubleSlash"'/g' $yamlPath
            break
        fi
    done
done