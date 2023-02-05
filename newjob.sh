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

    [[nodiscard]] auto getName() const -> std::string override {
        return getTypeNameByInst(*this);
    }

    void handle() override;

    ${JOBNAME}();
    ${JOBNAME}(${DEFAULTINPUT} inputdata);
};
}

EOF
)

cppsource=$(${COMMAND} <<EOF
#include "${JOBNAME}.hpp"
#include <iostream>

namespace job {
${JOBNAME}::${JOBNAME}() = default;

${JOBNAME}::${JOBNAME}(${DEFAULTINPUT} inputdata)
    : data(std::move(inputdata)) {}

void ${JOBNAME}::handle() {
    if (data.isNull()) {
        std::cout << "${JOBNAME} data is null" << std::endl;
        return;
    }

    data->stringify(std::cout);
    std::cout << std::endl;
}
}

EOF
)


echo "$cppheader" > "${DIR_SAIDA}/${JOBNAME}.hpp"
echo "$cppsource" > "${DIR_SAIDA}/${JOBNAME}.cpp"

