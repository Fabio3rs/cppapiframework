#!/bin/bash

DIR="jobs"
mkdir "autogen"

pushd "${DIR}"
FILES=$(find -iname '*.hpp')
popd

echo "$(clang-format - <<EOF
#pragma once
$(
for f in $FILES
do 
    CLASSNAME=$(echo $f | awk -F '.hpp' '{ print $1 }' | awk -F '/' '{ print $2 }')
	echo "#include \"../jobs/$CLASSNAME.hpp\""
done
)

namespace autogen{
inline void registerJobs(job::JobsHandler &handler){
    $(
for f in $FILES
do 
    CLASSNAME=$(echo $f | awk -F '.hpp' '{ print $1 }' | awk -F '/' '{ print $2 }')
	echo "handler.register_job_handler<job::$CLASSNAME>();"
done
)
}
};

EOF
)" > "autogen/jobslist.hpp"

