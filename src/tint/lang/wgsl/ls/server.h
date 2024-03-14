// Copyright 2024 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_TINT_LANG_WGSL_LS_SERVER_H_
#define SRC_TINT_LANG_WGSL_LS_SERVER_H_

#include <memory>

#include "langsvr/lsp/lsp.h"
#include "langsvr/session.h"
#include "src/tint/utils/text/string_stream.h"

namespace tint::wgsl::ls {

/// The language server state object.
class Server {
  public:
    /// Constructor
    /// @param session the LSP session.
    explicit Server(langsvr::Session& session);

    /// Destructor
    ~Server();

    /// @returns true if the server has been requested to shut down.
    bool ShuttingDown() const { return shutting_down_; }

  private:
    /// Logger is a string-stream like utility for logging to the client.
    /// Append message content with '<<'. The message is sent when the logger is destructed.
    struct Logger {
        ~Logger();

        /// @brief Appends @p value to the log message
        /// @return this logger
        template <typename T>
        Logger& operator<<(T&& value) {
            msg << value;
            return *this;
        }

        langsvr::Session& session;
        StringStream msg{};
    };

    /// Log constructs a new Logger to send a log message to the client.
    Logger Log() { return Logger{session_}; }

    /// The LSP session.
    langsvr::Session& session_;
    /// True if the server has been asked to shutdown.
    bool shutting_down_ = false;
};

}  // namespace tint::wgsl::ls

#endif  // SRC_TINT_LANG_WGSL_LS_SERVER_H_
