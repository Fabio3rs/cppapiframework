#pragma once

//#include "../TimeHandler.h"
#include <Poco/Net/HTMLForm.h>
#include <Poco/Net/MailMessage.h>
#include <sstream>

namespace mailbase {

struct Mail {

    void build() {
        /*Poco::Net::MailMessage message;

        std::stringstream sstr;

        TimeHandler msg;
        Poco::Net::HTMLForm form;
        msg.stringify(sstr, form);

        message.setContentType("text/html; charset=UTF-8");
        message.setContent(sstr.str(), Poco::Net::MailMessage::ENCODING_8BIT);

        message.setSubject("subject");
        message.write(std::cout);
        std::cout << std::endl;*/
    }
};

} // namespace mailbase
