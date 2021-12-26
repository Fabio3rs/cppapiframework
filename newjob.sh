#!/bin/bash

JOBNAME="TesteJob"
DEFAULTINPUT="Poco::JSON::Object::Ptr"

if [ $# -le 0 ];
then
    echo "Precisa de um argumento: Nome da classe do job" 1>&2
    exit 1
else
    JOBNAME="$1";
fi


if [ $# -gt 1 ];
then
    DEFAULTINPUT="$2";
fi

COMMAND="clang-format -"
DIR_SAIDA="src/jobs"

echo "Nome do job ${JOBNAME}"
echo "Diretório arquivos ${DIR_SAIDA}"
echo "Tipo de entrada de dados ${DEFAULTINPUT}"

cppheader=$(${COMMAND} <<EOF
#pragma once

#include "../jobhandler/JobsHandler.hpp"
#include "../jobhandler/QueueableJob.hpp"

namespace job {
class ${JOBNAME} : public QueueableJob {
    ${DEFAULTINPUT} data;

  public:
    QUEUEABLE_SERIALIZE(data)

    auto getName() const -> std::string_view override {
        return getTypeNameByInst(*this);
    }

    void handle() override;

    ${JOBNAME}();
    ${JOBNAME}(const ${DEFAULTINPUT} &inputdata);
};
}

EOF
)

cppsource=$(${COMMAND} <<EOF
#include "${JOBNAME}.hpp"
#include <iostream>

namespace job {
${JOBNAME}::${JOBNAME}() : QueueableJob() {}

${JOBNAME}::${JOBNAME}(const ${DEFAULTINPUT} &inputdata)
    : QueueableJob(), data(inputdata) {}

void ${JOBNAME}::handle() {
    if (data) {
        data->stringify(std::cout, 5);
        std::cout << std::endl;
    } else {
        std::cout << "${JOBNAME} data is null" << std::endl;
    }
}
}

EOF
)


echo "$cppheader" > "${DIR_SAIDA}/${JOBNAME}.hpp"
echo "$cppsource" > "${DIR_SAIDA}/${JOBNAME}.cpp"

