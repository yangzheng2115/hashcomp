// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <ostream>

#include "../misc/async.h"
#include "../misc/lss_allocator.h"

using namespace FASTER::core;

namespace FASTER {
    namespace io {

        enum class FileCreateDisposition : uint8_t {
            /// Creates the file if it does not exist; truncates it if it does.
                    CreateOrTruncate,
            /// Opens the file if it exists; creates it if it does not.
                    OpenOrCreate,
            /// Opens the file if it exists.
                    OpenExisting
        };

        inline std::ostream &operator<<(std::ostream &os, FileCreateDisposition val) {
            switch (val) {
                case FileCreateDisposition::CreateOrTruncate:
                    os << "CreateOrTruncate";
                    break;
                case FileCreateDisposition::OpenOrCreate:
                    os << "OpenOrCreate";
                    break;
                case FileCreateDisposition::OpenExisting:
                    os << "OpenExisting";
                    break;
                default:
                    os << "UNKNOWN: " << static_cast<uint8_t>(val);
                    break;
            }
            return os;
        }

        enum class FileOperationType : uint8_t {
            Read, Write
        };

        struct FileOptions {
            FileOptions()
                    : unbuffered{false}, delete_on_close{false} {
            }

            FileOptions(bool unbuffered_, bool delete_on_close_)
                    : unbuffered{unbuffered_}, delete_on_close{delete_on_close_} {
            }

            bool unbuffered;
            bool delete_on_close;
        };

    }
} // namespace FASTER::environment