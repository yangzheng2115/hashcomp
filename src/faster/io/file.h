// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#if (defined(_WIN32) || defined(__MINGW64__) || defined(__CYGWIN__))
#include "file_windows.h"
#else

#include "file_linux.h"

#endif
