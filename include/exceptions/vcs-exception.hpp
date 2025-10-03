#ifndef VCS_EXCEPTION_HPP
#define VCS_EXCEPTION_HPP

#include <exception>
#include <string>

class VCSException : public std::exception {
protected:
    std::string message;

public:
    explicit VCSException(const std::string& msg);
    const char* what() const noexcept override;
};

#endif // VCS_EXCEPTION_HPP
