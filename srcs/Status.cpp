#include "Status.hpp"

Status Status::OK(void) { return Status("");}

Status Status::Error(const std::string& message) { return Status(message); }

bool Status::ok(void) const { return _message.empty(); }

const std::string& Status::message(void) const { return _message; }

Status::Status(const std::string& message): _message(message) {}