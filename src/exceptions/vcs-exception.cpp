#include "exceptions/vcs-exception.hpp"

VCSException::VCSException(const std::string& msg) : message(msg) {}

const char* VCSException::what() const noexcept {
    return message.c_str();
}
